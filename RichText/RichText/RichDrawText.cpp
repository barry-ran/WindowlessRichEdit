#include "RichDrawText.h"
#include <assert.h>

#include "gdialpha.h"

EXTERN_C const IID IID_ITextServices = {
	0x8d33f740,
	0xcf58,
	0x11ce,
	{ 0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5 }
};

EXTERN_C const IID IID_ITextHost = {
	0xc5bdd8d0,
	0xd26e,
	0x11ce,
	{ 0xa8, 0x9e, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5 }
};

HMODULE RichDrawText::s_hRiched = nullptr;


void RichDrawText::SetText(LPCWSTR lpText)
{		
	m_textService->TxSetText(lpText);	
}

void RichDrawText::DrawText(HDC dc, const RECT& rect)
{
	// Draw the text in the windowless control onto the given device context,
  // within the given bounding rectangle.
	
	m_rcText = rect;
	// Alpha修复，否则会导致文字穿透
	ALPHAINFO ai;
	CGdiAlpha::AlphaBackup(dc, &rect, ai);

	RECTL rc = { rect.left, rect.top, rect.right, rect.bottom };
	HRESULT hr = m_textService->TxDraw(DVASPECT_CONTENT, 0, NULL, NULL, dc, 0,
		&rc, NULL, (PRECT)&rect, NULL, 0, TXTVIEW_ACTIVE);
	assert(SUCCEEDED(hr));
	CGdiAlpha::AlphaRestore(ai);
	
}


void RichDrawText::Range(long cpFirst, long cpLim, ITextRange** ppRange)
{
	// 检索文档活动故事中指定范围内容的文本范围对象。
	m_textDoc->Range(cpFirst, cpLim, ppRange);
}


void RichDrawText::SizeText(HDC dc, RECT& rect)
{
	LONG w = rect.right - rect.left;
	LONG h = 0;

	// Given a rectange of a particular width, work out how much vertical space
	// is needed to display the text in the windowless control.
	SIZEL extent = { -1, -1 };
	HRESULT hr = m_textService->TxGetNaturalSize(DVASPECT_CONTENT,
		dc, 0, NULL, TXTNS_FITTOCONTENT, &extent, &w, &h);
	
	if (FAILED(hr))
		h = 0;

	// Adjust the height of the rectangle
	rect.bottom = rect.top + h;
}


HRESULT RichDrawText::TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult)
{
	return m_textService->TxSendMessage(msg, wparam, lparam, plresult);
}


ITextServices* RichDrawText::GetTextDocument()
{
	return m_textService;
}

HRESULT STDMETHODCALLTYPE RichDrawText::QueryInterface(REFIID riid, void **ppvObject)
{
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	if ((riid == IID_IUnknown) || (riid == IID_ITextHost)) {
		*ppvObject = this;
		return S_OK;
	}

	*ppvObject = nullptr;
	return E_NOINTERFACE;
}


ULONG STDMETHODCALLTYPE RichDrawText::AddRef(void)
{
	return 0;
}


ULONG STDMETHODCALLTYPE RichDrawText::Release(void)
{
	return 0;
}

typedef HRESULT(_stdcall*CreateTextServicesFunction)(IUnknown*, ITextHost*, IUnknown**);

RichDrawText::RichDrawText(HDC hDC)
{		
	m_hDC = hDC;
	// Get the current font settings
	NONCLIENTMETRICSW ncm;
	::ZeroMemory(&ncm, sizeof ncm);
	ncm.cbSize = sizeof ncm;
	::SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof ncm, &ncm, 0);

	HDC hDC2 = ::GetDC(NULL);
	int iPointSize = -1 * ::MulDiv(ncm.lfMessageFont.lfHeight, 72, ::GetDeviceCaps(hDC2, LOGPIXELSY));
	::ReleaseDC(NULL, hDC2);
	
	// Create a default character format
	::ZeroMemory(&m_charFormat, sizeof(m_charFormat));
	m_charFormat.cbSize = sizeof(m_charFormat);
	// CFM_LINK must for support link
	m_charFormat.dwMask = CFM_BOLD | CFM_CHARSET | CFM_COLOR | CFM_FACE | CFM_ITALIC | CFM_OFFSET |
		CFM_PROTECTED | CFM_SIZE | CFM_STRIKEOUT |  CFM_LINK | CFM_LINKPROTECTED | CFM_UNDERLINETYPE;
	m_charFormat.bUnderlineType = CFU_UNDERLINENONE;	
	// CFE_LINK must for support link
	// CFE_LINKPROTECTED 汉字是友好名称链接的一部分
	m_charFormat.dwEffects = CFE_LINK | CFE_LINKPROTECTED | CFE_UNDERLINE;
	// 20*72=1440 ？duilib is 1440 too
	m_charFormat.yHeight = 20 * iPointSize;
	m_charFormat.crTextColor = RGB(255, 255, 255);
	m_charFormat.bPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;	
	wcscpy(m_charFormat.szFaceName, ncm.lfMessageFont.lfFaceName);

	// Create a default paragraph format
	::ZeroMemory(&m_paraFormat, sizeof(m_paraFormat));
	m_paraFormat.cbSize = sizeof(m_paraFormat);
	m_paraFormat.dwMask = PFM_ALIGNMENT | PFM_NUMBERING | PFM_OFFSET | PFM_OFFSETINDENT |
		PFM_RIGHTINDENT | PFM_RTLPARA | PFM_STARTINDENT | PFM_TABSTOPS;
	m_paraFormat.wAlignment = PFA_LEFT;

	if (s_hRiched)
	{
		//const IID* iidTextService = reinterpret_cast<IID*>(GetProcAddress(s_hRiched, "IID_ITextServices"));
		CreateTextServicesFunction createTextServices = reinterpret_cast<CreateTextServicesFunction>(GetProcAddress(s_hRiched, "CreateTextServices"));
		if (createTextServices)
		{
			CComPtr<IUnknown> unknown;
			HRESULT hr = createTextServices(nullptr, this, &unknown);
			assert(SUCCEEDED(hr));
			//unknown->QueryInterface(*iidTextService, reinterpret_cast<void**>(&m_textService));			
			m_textService = unknown;
			m_textDoc = unknown;

			LRESULT lResult;
			// ENM_LINK must for link
			HRESULT re = m_textService->TxSendMessage(EM_SETEVENTMASK, 0, ENM_LINK, &lResult);
			//re = m_textService->TxSendMessage(EM_SETEDITSTYLE, 0, SES_EX_HANDLEFRIENDLYURL, &lResult);
			//re = m_textService->TxSendMessage(EM_AUTOURLDETECT, FALSE, 0, &lResult);
			
			// must for link
			m_textService->OnTxInPlaceActivate(nullptr);
						
			//m_textService->OnTxPropertyBitsChange(TXTBIT_BACKSTYLECHANGE, 0);
		}
	}
}

BOOL RichDrawText::Init()
{
	s_hRiched = LoadLibrary(_T("msftedit.dll"));
	//s_hRiched = LoadLibrary(_T("riched20.dll"));
	if (!s_hRiched)
	{
		return FALSE;
	}
	return TRUE;
}

HDC RichDrawText::TxGetDC()
{
	// must for support link
	return m_hDC;
}

INT RichDrawText::TxReleaseDC(HDC hdc)
{
	// 1:success Release
	return 1;
}

BOOL RichDrawText::TxShowScrollBar(INT fnBar, BOOL fShow)
{
	return FALSE;
}

BOOL RichDrawText::TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags)
{
	return FALSE;
}

BOOL RichDrawText::TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw)
{
	return FALSE;
}

BOOL RichDrawText::TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw)
{
	return FALSE;
}

void RichDrawText::TxInvalidateRect(LPCRECT prc, BOOL fMode)
{

}

void RichDrawText::TxViewChange(BOOL fUpdate)
{

}

BOOL RichDrawText::TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight)
{
	return FALSE;
}

BOOL RichDrawText::TxShowCaret(BOOL fShow)
{
	return FALSE;
}

BOOL RichDrawText::TxSetCaretPos(INT x, INT y)
{
	return FALSE;
}

BOOL RichDrawText::TxSetTimer(UINT idTimer, UINT uTimeout)
{
	return FALSE;
}

void RichDrawText::TxKillTimer(UINT idTimer)
{

}

void RichDrawText::TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll)
{

}

void RichDrawText::TxSetCapture(BOOL fCapture)
{
	
}

void RichDrawText::TxSetFocus()
{
	
}

void RichDrawText::TxSetCursor(HCURSOR hcur, BOOL fText)
{
	
}

BOOL RichDrawText::TxScreenToClient(LPPOINT lppt)
{
	return FALSE;
}

BOOL RichDrawText::TxClientToScreen(LPPOINT lppt)
{
	return FALSE;
}

HRESULT RichDrawText::TxActivate(LONG * plOldState)
{
	return E_FAIL;
}

HRESULT RichDrawText::TxDeactivate(LONG lNewState)
{
	return E_FAIL;
}

HRESULT RichDrawText::TxGetClientRect(LPRECT prc)
{
	*prc = m_rcText;
	return S_OK;
}

HRESULT RichDrawText::TxGetViewInset(LPRECT prc)
{
	// Set zero sized margins
	prc->left = 0;
	prc->right = 0;
	prc->top = 0;
	prc->bottom = 0;
	return S_OK;
}

HRESULT RichDrawText::TxGetCharFormat(const CHARFORMATW **ppCF)
{
	*ppCF = &m_charFormat;
	return S_OK;
}

HRESULT RichDrawText::TxGetParaFormat(const PARAFORMAT **ppPF)
{
	*ppPF = &m_paraFormat;
	return S_OK;
}

COLORREF RichDrawText::TxGetSysColor(int nIndex)
{
	// Pass requests for colours on to Windows
	return ::GetSysColor(nIndex);
}

HRESULT RichDrawText::TxGetBackStyle(TXTBACKSTYLE *pstyle)
{
	// Do not erase what is underneath the drawing area
	*pstyle = TXTBACK_TRANSPARENT;
	//*pstyle = TXTBACK_OPAQUE;
	return S_OK;
}

HRESULT RichDrawText::TxGetMaxLength(DWORD *plength)
{
	// Set the maximum size of text to be arbitrarily large
	*plength = 1024 * 1024 * 16;
	return S_OK;
}

HRESULT RichDrawText::TxGetScrollBars(DWORD *pdwScrollBar)
{
	// Do not allow scrollbars
	*pdwScrollBar = 0;
	return S_OK;
}

HRESULT RichDrawText::TxGetPasswordChar(_Out_ TCHAR *pch)
{
	return S_FALSE;
}

HRESULT RichDrawText::TxGetAcceleratorPos(LONG *pcp)
{
	*pcp = -1;
	return S_OK;
}

HRESULT RichDrawText::TxGetExtent(LPSIZEL lpExtent)
{
	return E_NOTIMPL;
}

HRESULT RichDrawText::OnTxCharFormatChange(const CHARFORMATW * pCF)
{
	return E_FAIL;
}

HRESULT RichDrawText::OnTxParaFormatChange(const PARAFORMAT * pPF)
{
	return E_FAIL;
}

HRESULT RichDrawText::TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits)
{
	// Set the windowless control as being multiple lines of wrapping rich text
	// TXTBIT_RICHTEXT 富文本
	DWORD bits = TXTBIT_MULTILINE | TXTBIT_RICHTEXT | TXTBIT_WORDWRAP;
	*pdwBits = bits & dwMask;
	return S_OK;
}

HRESULT RichDrawText::TxNotify(DWORD iNotify, void *pv)
{
	switch (iNotify)
	{
	case EN_LINK:
	{
		ENLINK *bnlink = (ENLINK *)pv;		
		if (bnlink && bnlink->msg == WM_LBUTTONDOWN)
		{
			//Call ShellExecute to perform default action based on the type of hyperlink

			WCHAR wszLink[1024] = { 0 };
			LRESULT lResult = 0;
			TEXTRANGEW tr;
			tr.chrg = bnlink->chrg;
			tr.lpstrText = wszLink;
			// 对于友好名称超链接，这行代码获取的是超链接本身，而不是友好名称，所以可以直接ShellExecuteW，完美
			m_textService->TxSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&tr, &lResult);
			
			::OutputDebugStringW(wszLink);

			ShellExecuteW(nullptr, L"Open", wszLink, NULL, NULL, SW_MAXIMIZE);
		}
	}
		break;
	case EN_SETFOCUS:
		::OutputDebugStringA("EN_SETFOCUS\n");
		break;
	}

	// Claim to have handled the notifcation, even though we always ignore it
	return S_OK;
}

HIMC RichDrawText::TxImmGetContext()
{
	return 0;
}

void RichDrawText::TxImmReleaseContext(HIMC himc)
{
	
}

HRESULT RichDrawText::TxGetSelectionBarWidth(LONG *lSelBarWidth)
{
	// No selection bar
	*lSelBarWidth = 0;
	return S_OK;
}
