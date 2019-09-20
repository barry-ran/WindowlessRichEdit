// RichText.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "RichText.h"

#include <stdio.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

#include "RichDrawText.h"

RichDrawText *g_richDrawText = nullptr;

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[] = _T("PNGDialog");

int WINAPI WinMain(HINSTANCE hThisInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpszArgument,
	int nCmdShow)
{
	/**/
	/*  GDI+ startup token */
	ULONG_PTR gdiplusStartupToken;
	Gdiplus::GdiplusStartupInput gdiInput;
	Gdiplus::GdiplusStartup(&gdiplusStartupToken, &gdiInput, NULL);	

	MSG messages;            /* Here messages to the application are saved */
	{
		/**/
		HWND hwnd;               /* This is the handle for our window */
		WNDCLASSEX wincl;        /* Data structure for the windowclass */

		/* The Window structure */
		wincl.hInstance = hThisInstance;
		wincl.lpszClassName = szClassName;//+-69+
		wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
		wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
		wincl.cbSize = sizeof(WNDCLASSEX);

		/* Use default icon and mouse-pointer */
		wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
		wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
		wincl.lpszMenuName = NULL;                 /* No menu */
		wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
		wincl.cbWndExtra = 0;                      /* structure or the window instance */
		/* Use Windows's default colour as the background of the window */
		wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

		/* Register the window class, and if it fails quit the program */
		if (!RegisterClassEx(&wincl))
			return 0;

		/* The class is registered, let's create the program*/
		// mark:WS_EX_LAYERED
		hwnd = CreateWindowEx(
			WS_EX_LAYERED,                   /* Extended possibilites for variation */
			szClassName,         /* Classname */
			_T("PNG transparent"),       /* Title Text */
			WS_POPUP, /* default window */
			CW_USEDEFAULT,       /* Windows decides the position */
			CW_USEDEFAULT,       /* where the window ends up on the screen */
			376,                 /* The programs width */
			72,                 /* and height in pixels */
			HWND_DESKTOP,        /* The window is a child-window to desktop */
			NULL,                /* No menu */
			hThisInstance,       /* Program Instance handler */
			NULL                 /* No Window Creation data */
		);				

		// 获取系统字体
		NONCLIENTMETRICS metrics;
		metrics.cbSize = sizeof(NONCLIENTMETRICS);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS),
			&metrics, 0);
		HFONT font = ::CreateFontIndirect(&metrics.lfMessageFont);					
		
		RECT wndRect;
		::GetWindowRect(hwnd, &wndRect);
		SIZE wndSize = { wndRect.right - wndRect.left,wndRect.bottom - wndRect.top };
		HDC hdc = ::GetDC(hwnd);
		HDC memDC = ::CreateCompatibleDC(hdc);
		// mark:HBITMAP不用CreateCompatibleBitmap来创建，因为这样创建的HBITMAP没法操作内存数据，后面我们要通过修改内存修复gdi导致alpha通道问题
		//HBITMAP memBitmap = ::CreateCompatibleBitmap(hdc, wndSize.cx, wndSize.cy);

		// mark:CreateDIBSection创建HBITMAP可以操作内存数据
		BITMAPINFO bitmapinfo;
		bitmapinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bitmapinfo.bmiHeader.biBitCount = 32;
		bitmapinfo.bmiHeader.biHeight = wndSize.cy;
		bitmapinfo.bmiHeader.biWidth = wndSize.cx;
		bitmapinfo.bmiHeader.biPlanes = 1;
		bitmapinfo.bmiHeader.biCompression = BI_RGB;
		bitmapinfo.bmiHeader.biXPelsPerMeter = 0;
		bitmapinfo.bmiHeader.biYPelsPerMeter = 0;
		bitmapinfo.bmiHeader.biClrUsed = 0;
		bitmapinfo.bmiHeader.biClrImportant = 0;
		bitmapinfo.bmiHeader.biSizeImage =
		bitmapinfo.bmiHeader.biWidth * bitmapinfo.bmiHeader.biHeight *
		bitmapinfo.bmiHeader.biBitCount / 8;
		HBITMAP memBitmap = ::CreateDIBSection(memDC, &bitmapinfo, 0, NULL, 0, 0);

		::SelectObject(memDC, memBitmap);		

		Gdiplus::Image image(L"bg.png");
		Gdiplus::Graphics graphics(memDC);
		graphics.DrawImage(&image, 0, 0, wndSize.cx, wndSize.cy);

		RichDrawText::Init();
		g_richDrawText = new RichDrawText(memDC);		

		// 友好名称超链接 https://blogs.msdn.microsoft.com/murrays/2009/09/24/richedit-friendly-name-hyperlinks/
		// 友好名称的下划线如何删除？颜色如何修改？
		g_richDrawText->SetText(L"{\\rtf1 超链接{\\field{\\*\\fldinst HYPERLINK \"https://www.baidu.com/\"}{\\fldrslt 百度百度百度百度}} }");


		// 前两个字符红色
		CComPtr<ITextRange> range;
		g_richDrawText->Range(0, 2, &range);

		CComPtr<ITextFont> textFont;
		range->GetFont(&textFont);
		textFont->SetForeColor(RGB(255, 0, 0));


		RECT rcText;
		::GetClientRect(hwnd, &rcText);

		rcText.left = 20;
		rcText.top = 20;
		rcText.right = 300;
		rcText.bottom = 50;
		g_richDrawText->DrawText(memDC, rcText);
		
		POINT ptSrc = { 0,0 };
		
		BLENDFUNCTION blendFunction;
		blendFunction.AlphaFormat = AC_SRC_ALPHA;
		blendFunction.BlendFlags = 0;
		blendFunction.BlendOp = AC_SRC_OVER;
		blendFunction.SourceConstantAlpha = 255;
		// mark:UpdateLayeredWindow png图片自带透明通道配合ULW_ALPHA标记，效果好
		// ULW_COLORKEY适合将指定颜色透明
		::UpdateLayeredWindow(hwnd, hdc, &ptSrc, &wndSize, memDC, &ptSrc, 0, &blendFunction, ULW_ALPHA);

		::ReleaseDC(hwnd, hdc);
		::DeleteDC(memDC);
		::DeleteObject(memBitmap);
		::DeleteObject(font);

		::SetWindowPos(hwnd, NULL, 500, 500, 0, 0, SWP_NOSIZE);
		/* Make the window visible on the screen */
		::ShowWindow(hwnd, nCmdShow);

		/* Run the message loop. It will run until GetMessage() returns 0 */
		while (GetMessage(&messages, NULL, 0, 0))
		{
			/* Translate virtual-key messages into character messages */
			TranslateMessage(&messages);
			/* Send message to WindowProcedure */
			DispatchMessage(&messages);
		}
	}
	Gdiplus::GdiplusShutdown(gdiplusStartupToken);
	/* The program return-value is 0 - The value that PostQuitMessage() gave */
	return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	switch (message)                  /* handle the messages */
	{
	case WM_DESTROY:
		PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
		break;	
	case WM_SETFOCUS:
		//break;
		if (g_richDrawText)
		{
			//g_richDrawText->GetTextDocument()->OnTxInPlaceActivate(nullptr);

			LRESULT lResult = 0;
			// WM_SETFOCUS must for link mouse move
			HRESULT Hr = g_richDrawText->TxSendMessage(message, wParam, lParam, &lResult);			
			if (S_OK != Hr)
			{
				OutputDebugStringA("error\n");
			}
		}
		break;
	case WM_KILLFOCUS:		
		if (g_richDrawText)
		{
			//g_richDrawText->GetTextDocument()->OnTxInPlaceDeactivate();

			LRESULT lResult = 0;
			HRESULT Hr = g_richDrawText->TxSendMessage(message, wParam, lParam, &lResult);
			if (S_OK != Hr)
			{
				OutputDebugStringA("error\n");
			}
		}
		break;
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
		// must for link click
		if (g_richDrawText)
		{
			LRESULT lResult = 0;
			HRESULT Hr = g_richDrawText->TxSendMessage(message, wParam, lParam, &lResult);
			if (S_OK != Hr)
			{
				OutputDebugStringA("error\n");
			}
		}
		break;
	default:                      /* for messages that we don't deal with */
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}
