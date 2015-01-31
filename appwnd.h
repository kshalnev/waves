#pragma once

#include <vector>
#include <memory>

//
// CAppWindow class
//

class CAppWindow : public CWindowImpl<CAppWindow>
{
public:
    CAppWindow();
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

    inline void _UpdateBuffer();
    inline void _SwapBuffer();

private:
    class CParticleSystem
    {
    private:
        struct SParticle
        {
        public:
            long x; // X
            long y; // Y
            long amplitudex100; // amplitude to emulate attenuation at time, x100
            long phaseOffsetx100; // phase offset to emulate running wave, x100
            DWORD startTime; // start time to make delayed appearance
        };
    public:
        CParticleSystem(size_t count, long width, long height, DWORD time);
        ~CParticleSystem();

        inline COLORREF GetColor(long x, long y) const;
        inline void Update(DWORD time);

    private:
        inline void _InitParticle(SParticle& particle, DWORD time) const;

    private:
        const long m_width;
        const long m_doubleWidth;
        const long m_height;
        const long m_doubleHeight;
        const double m_amplitudeAttenuation;
        std::vector<SParticle> m_particles;
        std::vector<SParticle*> m_visibleParticles;
        std::vector<long> m_distCoefx100;
        std::vector<long> m_attenuationx100;
        std::vector<long> m_cosinex100;
    };

private:
    HDC m_hDC;
    HDC m_hMemDC;
    HBITMAP m_hMemBmp;
    HBITMAP m_hOldMemBmp;
    BITMAP m_memBmp;
    std::auto_ptr<CParticleSystem> m_apParticleSystem;
};
