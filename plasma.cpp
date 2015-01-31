#include "stdafx.h"
#include "appwnd.h"

class CMainWindow : public CAppWindow
{
public:
    CMainWindow(int nParticles = 0) 
      : CAppWindow(nParticles)
    {
    }
protected:
    virtual void OnFinalMessage(HWND)
    {
        ::PostQuitMessage(0);
    }
};

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpszCmdLine, int nCmdShow)
{
    int nParticles = 0;
    if ( lpszCmdLine && lpszCmdLine[0] )
      nParticles = _ttoi(lpszCmdLine);

    BOOL bRevertSettings = FALSE;

    RECT r = { 0 };
    DEVMODE dmOrigin = { 0 };
    if ( ::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dmOrigin) )
    {
        r.right = dmOrigin.dmPelsWidth;
        r.bottom = dmOrigin.dmPelsHeight; 

        DEVMODE dmNew = dmOrigin;
        dmNew.dmPelsWidth = 640;
        dmNew.dmPelsHeight = 480;
        if ( 0 && ::ChangeDisplaySettingsEx(NULL, &dmNew, NULL, 0, NULL) == DISP_CHANGE_SUCCESSFUL )
        {
            ::ShowCursor(FALSE);
            bRevertSettings = TRUE;

            r.right = dmNew.dmPelsWidth;
            r.bottom = dmNew.dmPelsHeight; 
        }
    }

    CMainWindow wnd(nParticles);
    HWND hWnd = wnd.Create(NULL, r, NULL, WS_POPUP);

    if ( hWnd != NULL )
    {
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
    }

    if ( bRevertSettings )
    {
      ::ChangeDisplaySettingsEx(NULL, &dmOrigin, NULL, 0, NULL);
      ::ShowCursor(TRUE);
    }

    return 0;
}
