#pragma once

#include <memory>
#include "particles.h"

//
// CAppWindow class
//

class CAppWindow : public CWindowImpl<CAppWindow>
{
public:
    CAppWindow(int nParticles = 0);
    ~CAppWindow();

    BEGIN_MSG_MAP(CAppWindow)
        MESSAGE_HANDLER(WM_CREATE, _OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, _OnDestroy)
        MESSAGE_HANDLER(WM_PAINT, _OnPaint)
        MESSAGE_HANDLER(WM_ERASEBKGND, _OnEraseBgnd)
    END_MSG_MAP()

    void Refresh();

private:
    LRESULT _OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT _OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT _OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT _OnEraseBgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    inline void _UpdateBuffer(HDC hDC, const BITMAP& bitmap);
    inline void _SwapBuffer();

private:
    HDC m_hDC;
    HDC m_hMemDC;
    HBITMAP m_hMemBmp;
    HBITMAP m_hOldMemBmp;
    BITMAP m_memBmp;
    CImage m_backgroundImage;
    int m_nParticles;
    std::auto_ptr<CParticleSystem> m_apParticleSystem;
};
