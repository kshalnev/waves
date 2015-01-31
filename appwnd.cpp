#include "stdafx.h"
#include "appwnd.h"

//
// Consts
//

const long NUMBER_OF_PARTICLES = 30;

//
// CAppWindow class
//

CAppWindow::CAppWindow(int nParticles /*= 0*/) 
{
    m_hDC = NULL;
    m_hMemDC = NULL;
    m_hMemBmp = NULL;
    m_hOldMemBmp = NULL;
    ZeroMemory(&m_memBmp, sizeof(m_memBmp));
    if (nParticles <= 0)
      m_nParticles = NUMBER_OF_PARTICLES;
    else
      m_nParticles = nParticles;
}

CAppWindow::~CAppWindow() 
{
}

void CAppWindow::Refresh()
{
    _UpdateBuffer(m_hMemDC, m_memBmp);
    _SwapBuffer();
}

LRESULT CAppWindow::_OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    m_backgroundImage.Load(_T("image.jpg"));
    SetWindowPos(NULL, 0, 0, m_backgroundImage.GetWidth(), m_backgroundImage.GetHeight(),
      SWP_NOZORDER|SWP_NOMOVE);
    CenterWindow();

    RECT rect;
    GetClientRect(&rect);

    BITMAPINFO bi = { 0 };
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = rect.right;
    bi.bmiHeader.biHeight = -rect.bottom;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    bi.bmiHeader.biSizeImage = 0;
    bi.bmiHeader.biXPelsPerMeter = 0;
    bi.bmiHeader.biYPelsPerMeter = 0;
    bi.bmiHeader.biClrUsed = 0;
    bi.bmiHeader.biClrImportant = 0;

    HDC hDC = GetDC();
    HDC hMemDC = CreateCompatibleDC(hDC);
    HBITMAP hMemBmp = CreateDIBSection(hDC, &bi, DIB_RGB_COLORS, NULL, NULL, 0);
    HBITMAP hOldMemBmp = (HBITMAP)SelectObject(hMemDC, hMemBmp);
    BITMAP bmp = { 0 };
    GetObject(hMemBmp, sizeof(bmp), &bmp);
    ZeroMemory(bmp.bmBits, bmp.bmWidthBytes * bmp.bmHeight);

    m_hDC = hDC;
    m_hMemDC = hMemDC;
    m_hMemBmp = hMemBmp;
    m_hOldMemBmp = hOldMemBmp;
    m_memBmp = bmp;

    m_apParticleSystem.reset( new CParticleSystem(m_nParticles, bmp.bmWidth, bmp.bmHeight) );

    return 0;
}

LRESULT CAppWindow::_OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SelectObject(m_hMemDC, m_hOldMemBmp);
    DeleteObject(m_hMemBmp);
    DeleteDC(m_hMemDC);
    ReleaseDC(m_hDC);

    m_hDC = NULL;
    m_hMemDC = NULL;
    m_hMemBmp = NULL;
    m_hOldMemBmp = NULL;
    ZeroMemory(&m_memBmp, sizeof(m_memBmp));
    
    return 0;
}
LRESULT CAppWindow::_OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    _UpdateBuffer(m_hMemDC, m_memBmp);
    _SwapBuffer();
    
    return 0;
}

LRESULT CAppWindow::_OnEraseBgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 1;
}

void CAppWindow::_UpdateBuffer(HDC hDC, const BITMAP& bitmap)
{   
    m_apParticleSystem->Update();
    m_apParticleSystem->Draw(m_backgroundImage, bitmap);
}

void CAppWindow::_SwapBuffer()
{
    BitBlt(m_hDC, 0, 0, m_memBmp.bmWidth, m_memBmp.bmHeight, m_hMemDC, 0, 0, SRCCOPY);
}
