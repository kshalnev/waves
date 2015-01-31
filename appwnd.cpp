#include "stdafx.h"
#include "appwnd.h"

//
// Consts
//

static const size_t PARTICLES_COUNT = 30;
static const double AMPLITUDE_ATTENUATION = 0.95;
static const DWORD MAX_TIME_DELAY_MILLISEC = 800;

static const double _PI = 3.1415926535897932384626433832795;
static const double DEG_TO_RAG = _PI / 180.0;
static const double RAD_TO_DEG = 180.0 / _PI;
static const double RAD_TO_DEG_x100 = RAD_TO_DEG * 100.0;

//
// CAppWindow::CParticleSystem class
//

CAppWindow::CParticleSystem::CParticleSystem(size_t count, long width, long height, DWORD time)
    : m_amplitudeAttenuation(AMPLITUDE_ATTENUATION)
    , m_width(width)
    , m_doubleWidth(width * 2)
    , m_height(height)
    , m_doubleHeight(height * 2)
{
    m_particles.resize(count);
    m_visibleParticles.resize(count + 1);
    m_visibleParticles[count] = NULL;
    for ( size_t i = 0; i < count; ++i )
    {   
        m_visibleParticles[i] = NULL;

        SParticle& particle = m_particles[i];
        // init particle
        _InitParticle(particle, time);
    }

    m_distCoefx100.resize(m_doubleWidth * m_doubleHeight);
    m_attenuationx100.resize(m_doubleWidth * m_doubleHeight);
    for ( long y = -m_height; y < m_height; ++y ) 
    {
        for ( long x = -m_width; x < m_width; ++x )
        {
            const double dist = _hypot(x, y);
            
            // calculate attennuation in distance like 1/d
            // note, when X is so small attennuation can be > 1 that incorrect
            // therefore apply attennuation since distance where 1/d = 1
            const double attn = min(0.33, (1.0 / dist));

            long index = (x + m_width) + (y + m_height) * m_doubleWidth;
            m_distCoefx100[index] = long((dist / 4.0) * RAD_TO_DEG_x100) + 36000 * 50;
            m_attenuationx100[index] = long(attn * 100);
        }
    }

    m_cosinex100.resize(360 * 100);
    for ( size_t a = 0; a < m_cosinex100.size(); ++a )
    {
        m_cosinex100[a] = long(0.5 + 100.0 * cos( double(a) * DEG_TO_RAG / 100.0 ));
    }
}

CAppWindow::CParticleSystem::~CParticleSystem()
{
}

COLORREF CAppWindow::CParticleSystem::GetColor(long x, long y) const
{
    register long value = 0;
    const SParticle* const* ppParticle = &m_visibleParticles[0];
    for ( ; *ppParticle != NULL; ++ppParticle )
    {
        const SParticle& particle = **ppParticle;

        const long cx = x - particle.x + m_width, cy = y - particle.y + m_height;
        const long index = cx + cy * m_doubleWidth;
        const long distCoefx100 = m_distCoefx100[index]; // distance between X,Y point and particle
        const long attnInDistx100 = m_attenuationx100[index]; // attennuation in distance

        // particle algorithms is:
        // this particle influenxe in X,Y point is
        // use COS since COS(0) = 1 to have particle wave peak at particle position
        // use COS of dist since we need 2D figure
        // also, accumulate influence from all visible particles
        // dist = hypot(x - particle.x, y - particle.y);
        // attenuationInDistance = min(1.0, 1.0 / dist);
        // value = particle.amplitude * attenuationInDistance * cos(dist / 4.0 - particle.phaseOffset);

        long anglex100 = distCoefx100 - particle.phaseOffsetx100;
        anglex100 %= 36000;
        ASSERT(anglex100 >= 0);

        value += particle.amplitudex100 * attnInDistx100 * m_cosinex100[anglex100];
    }    

    // evaluate pixel color
    const int color = int(127.0 + 0.000127 * value);
    ASSERT(0 <= color && color <= 255);

    return RGB(color, color, 64);
}

void CAppWindow::CParticleSystem::Update(DWORD time)
{
    size_t visibleIndex = 0;
    const size_t countParticles = m_particles.size();
    for ( size_t i = 0; i < countParticles; ++i )
    {
        SParticle& particle = m_particles[i];
        // if particle was burn and its amplitude not so small yet
        // then make it visible, otherwise re-create particle
        if ( (particle.amplitudex100 > 1) && (time > particle.startTime) )
        {
            // make amplitude attenuation
            particle.amplitudex100 = long(particle.amplitudex100 * m_amplitudeAttenuation); 
            
            // make phase offset
            particle.phaseOffsetx100 = long(RAD_TO_DEG * (time - particle.startTime)); 

            m_visibleParticles[visibleIndex] = &particle;
            ++visibleIndex;
        }
        else
        {
            // re-init particle
            _InitParticle(particle, time);
        }
    }

    m_visibleParticles[visibleIndex] = NULL;
}

void CAppWindow::CParticleSystem::_InitParticle(SParticle& particle, DWORD time) const
{
    particle.x = 30 + rand() % (m_width - 60);
    particle.y = 30 + rand() % (m_height - 60);
    particle.amplitudex100 = 100;
    particle.phaseOffsetx100 = 0;
    particle.startTime = time + rand() % MAX_TIME_DELAY_MILLISEC;
}

//
// CAppWindow class
//

CAppWindow::CAppWindow() 
{
    m_hDC = NULL;
    m_hMemDC = NULL;
    m_hMemBmp = NULL;
    m_hOldMemBmp = NULL;
    ZeroMemory(&m_memBmp, sizeof(m_memBmp));
}

CAppWindow::~CAppWindow() 
{
}

void CAppWindow::Refresh()
{
    _UpdateBuffer();
    _SwapBuffer();
}

LRESULT CAppWindow::_OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
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

    m_apParticleSystem = std::auto_ptr<CParticleSystem>(
        new CParticleSystem(PARTICLES_COUNT, bmp.bmWidth, bmp.bmHeight, GetTickCount()));

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
    _UpdateBuffer();
    _SwapBuffer();
    
    return 0;
}

LRESULT CAppWindow::_OnEraseBgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 1;
}

void CAppWindow::_SwapBuffer()
{
    BitBlt(m_hDC, 0, 0, m_memBmp.bmWidth, m_memBmp.bmHeight, m_hMemDC, 0, 0, SRCCOPY);
}

void CAppWindow::_UpdateBuffer()
{
    m_apParticleSystem->Update(GetTickCount());

    const long w = m_memBmp.bmWidth;
    const long h = m_memBmp.bmHeight;
    BYTE* pbPixelLine = (BYTE*)m_memBmp.bmBits;
    for ( long y = 0; y < h; ++y, pbPixelLine += m_memBmp.bmWidthBytes )
    {
        COLORREF* pPixel = (COLORREF*)pbPixelLine;
        for ( long x = 0; x < w; ++x, ++pPixel )
        {
            *pPixel = m_apParticleSystem->GetColor(x, y);
        }
    }
}
