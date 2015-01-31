#include "stdafx.h"
#include "appwnd.h"

// #ifdef _DEBUG
#define _SMALLSCREEN_DEBUG
// #endif

class CMainWindow : public CAppWindow
{
protected:
    virtual void OnFinalMessage(HWND)
    {
        ::PostQuitMessage(0);
    }
};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpszCmdLine, int nCmdShow)
{
    #ifdef _SMALLSCREEN_DEBUG
    BOOL bAdjustDisplaySettings = FALSE;
    const LONG nTop = 400;
    const LONG nLeft = 400;
    const LONG nWidth = 640;
    const LONG nHeight = 480;
    #else
    BOOL bAdjustDisplaySettings = TRUE;
    const LONG nTop = 0;
    const LONG nLeft = 0;
    const LONG nWidth = 800;
    const LONG nHeight = 600;
    #endif

    RECT r = { nLeft, nTop, nLeft + nWidth, nTop + nHeight };

    CMainWindow wnd;
    HWND hWnd = wnd.Create(NULL, r, NULL, WS_POPUP);

    if ( hWnd != NULL )
    {
        BOOL bRevertSettings = FALSE;

        DEVMODE dmOrigin = { 0 };
        BOOL bOrigin = ::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmOrigin);

        if ( bAdjustDisplaySettings && bOrigin && 
            (dmOrigin.dmPelsWidth != nWidth || dmOrigin.dmPelsHeight != nHeight) )
        {
            DEVMODE dmNew = dmOrigin;
            dmNew.dmPelsWidth = nWidth;
            dmNew.dmPelsHeight = nHeight;
            if ( ::ChangeDisplaySettingsEx(NULL, &dmNew, NULL, 0, NULL) == DISP_CHANGE_SUCCESSFUL )
            {
                ::ShowCursor(FALSE);
                bRevertSettings = TRUE;
            }
        }

        ::ShowWindow(hWnd, SW_SHOW);

        while ( ::IsWindow(hWnd) )
        {
            MSG msg;
            if ( ::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
            else
            {
                wnd.Refresh();
            }
        }

        hWnd = NULL;

        if ( bRevertSettings )
        {
            ::ChangeDisplaySettingsEx(NULL, &dmOrigin, NULL, 0, NULL);
            ::ShowCursor(TRUE);
        }
    }

    return 0;
}
