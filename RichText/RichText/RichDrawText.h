#ifndef _RICH_DRAW_TEXT_H_
#define _RICH_DRAW_TEXT_H_

#include <Richedit.h>
#include <atlbase.h>
#include <TextServ.h>
#include <tom.h>

class RichDrawText : public ITextHost
{
public:
	RichDrawText(HDC hDC);

	static BOOL Init();	

	static HMODULE s_hRiched;

	void SetText(LPCWSTR lpText);
	void DrawText(HDC dc, const RECT& rect);
	void Range(long cpFirst, long cpLim, ITextRange** ppRange);
	void SizeText(HDC dc, RECT& rect);
	void SetDC(HDC hdc);
	HRESULT TxSendMessage(UINT msg, WPARAM wparam, LPARAM lparam, LRESULT *plresult);
	ITextServices* GetTextDocument();
	
protected:
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);


	virtual ULONG STDMETHODCALLTYPE AddRef(void);


	virtual ULONG STDMETHODCALLTYPE Release(void);

	virtual HDC TxGetDC() override;


	virtual INT TxReleaseDC(HDC hdc) override;


	virtual BOOL TxShowScrollBar(INT fnBar, BOOL fShow) override;


	virtual BOOL TxEnableScrollBar(INT fuSBFlags, INT fuArrowflags) override;


	virtual BOOL TxSetScrollRange(INT fnBar, LONG nMinPos, INT nMaxPos, BOOL fRedraw) override;


	virtual BOOL TxSetScrollPos(INT fnBar, INT nPos, BOOL fRedraw) override;


	virtual void TxInvalidateRect(LPCRECT prc, BOOL fMode) override;


	virtual void TxViewChange(BOOL fUpdate) override;


	virtual BOOL TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight) override;


	virtual BOOL TxShowCaret(BOOL fShow) override;


	virtual BOOL TxSetCaretPos(INT x, INT y) override;


	virtual BOOL TxSetTimer(UINT idTimer, UINT uTimeout) override;


	virtual void TxKillTimer(UINT idTimer) override;


	virtual void TxScrollWindowEx(INT dx, INT dy, LPCRECT lprcScroll, LPCRECT lprcClip, HRGN hrgnUpdate, LPRECT lprcUpdate, UINT fuScroll) override;


	virtual void TxSetCapture(BOOL fCapture) override;


	virtual void TxSetFocus() override;


	virtual void TxSetCursor(HCURSOR hcur, BOOL fText) override;


	virtual BOOL TxScreenToClient(LPPOINT lppt) override;


	virtual BOOL TxClientToScreen(LPPOINT lppt) override;


	virtual HRESULT TxActivate(LONG * plOldState) override;


	virtual HRESULT TxDeactivate(LONG lNewState) override;


	virtual HRESULT TxGetClientRect(LPRECT prc) override;


	virtual HRESULT TxGetViewInset(LPRECT prc) override;


	virtual HRESULT TxGetCharFormat(const CHARFORMATW **ppCF) override;


	virtual HRESULT TxGetParaFormat(const PARAFORMAT **ppPF) override;


	virtual COLORREF TxGetSysColor(int nIndex) override;


	virtual HRESULT TxGetBackStyle(TXTBACKSTYLE *pstyle) override;


	virtual HRESULT TxGetMaxLength(DWORD *plength) override;


	virtual HRESULT TxGetScrollBars(DWORD *pdwScrollBar) override;


	virtual HRESULT TxGetPasswordChar(_Out_ TCHAR *pch) override;


	virtual HRESULT TxGetAcceleratorPos(LONG *pcp) override;


	virtual HRESULT TxGetExtent(LPSIZEL lpExtent) override;


	virtual HRESULT OnTxCharFormatChange(const CHARFORMATW * pCF) override;


	virtual HRESULT OnTxParaFormatChange(const PARAFORMAT * pPF) override;


	virtual HRESULT TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits) override;


	virtual HRESULT TxNotify(DWORD iNotify, void *pv) override;


	virtual HIMC TxImmGetContext() override;


	virtual void TxImmReleaseContext(HIMC himc) override;


	virtual HRESULT TxGetSelectionBarWidth(LONG *lSelBarWidth) override;

private:
	CHARFORMAT2W m_charFormat;
	PARAFORMAT2 m_paraFormat;

	//CHARFORMATW m_charFormat;
	//PARAFORMAT m_paraFormat;

	CComQIPtr<ITextServices, &IID_ITextServices> m_textService;
	CComQIPtr<ITextDocument> m_textDoc;

	RECT m_rcText;
	HDC m_hDC;
};

#endif //_RICH_DRAW_TEXT_H_
