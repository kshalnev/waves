#include "stdafx.h"
#include "particles.h"

//
// Consts
//

static const double _PI = 3.1415926535897932384626433832795;
static const double DEG_TO_RAG = _PI / 180.0;
static const double RAD_TO_DEG = 180.0 / _PI;

static const double AMPLITUDE_ATTENUATION = 0.9;
static const long AMPLITUDE_MINIMUM_x100 = (long)( 100.0 * 0.5 );
static const long AMPLITUDE_MAXIMUM_x100 = (long)( 100.0 * 250.0 );
static const double FREQUENCY_RATIO = ( 1.0 / 2.0 ) * RAD_TO_DEG;
static const double PHASE_SHIFT_DEG = ( 0.5 ) * RAD_TO_DEG;
static const long PHASE_SHIFT_DEG_x100 = (long)( 100.0 * PHASE_SHIFT_DEG );

//
// CParticleSystem class
//

CParticleSystem::CParticleSystem(size_t count, long width, long height)
    : m_width(width)
    , m_doubleWidth(width * 2)
    , m_height(height)
    , m_doubleHeight(height * 2)
{
    // Init particles system and visible particles array
    m_particles.resize(count);
    m_visibleParticles.resize(count + 1);
    m_visibleParticles[count] = NULL;
    for ( size_t i = 0; i < count; ++i )
    {   
        m_visibleParticles[i] = NULL;

        SParticle& particle = m_particles[i];
        // init particle
        _InitParticle(particle);
    }

    // Init distance and attenuation
    // for quick calculation of distance and attenuation
    m_distanceCoefx100.resize(m_doubleWidth * m_doubleHeight);
    m_attenuationx100.resize(m_doubleWidth * m_doubleHeight);
    for ( long y = -m_height; y < m_height; ++y ) 
    {
        for ( long x = -m_width; x < m_width; ++x )
        {
            const double dist = _hypot(x, y);
            const double attn = min(1.0, (1.0 / dist));

            long index = (x + m_width) + (y + m_height) * m_doubleWidth;
            m_distanceCoefx100[index] = (long)(100.0 * dist * FREQUENCY_RATIO);
            m_attenuationx100[index] = (long)(100.0 * attn);
        }
    }

    // Init sine and cosine coefficients
    m_sineCoefx100.resize(m_doubleWidth * m_doubleHeight);
    m_cosineCoefx100.resize(m_doubleWidth * m_doubleHeight);
    for ( long y = -m_height, i = 0; y < m_height; ++y ) 
    {
        for ( long x = -m_width; x < m_width; ++x, ++i )
        {
            const double dist = _hypot(x, y);
            if ( dist != 0.0 )
            {
                m_cosineCoefx100[i] = (long)( 100.0 * (double)x / dist );
                m_sineCoefx100[i] = (long)( 100.0 * (double)y / dist );
            }
            else
            {
                m_cosineCoefx100[i] = m_sineCoefx100[i] = 0;
            }
        }
    }

    // Init cosine
    // for quick calculation of cosine
    m_cosinex100.resize(2 * 360 * 100);
    for ( size_t a = 0; a < m_cosinex100.size(); ++a )
    {
        m_cosinex100[a] = (long)( 100.0 * cos(double(a) * DEG_TO_RAG / 100.0) );
    }
}

CParticleSystem::~CParticleSystem()
{
}

void CParticleSystem::Draw(const CImage& bitmapSrc, const BITMAP& bitmapDst) const
{
    int maxXx10000 = 10000 * (bitmapDst.bmWidth - 1 - 1);
    int maxYx10000 = 10000 * (bitmapDst.bmHeight - 1 - 1);

    BYTE* pDestRow = (BYTE*)bitmapDst.bmBits;
    for ( int dy = 0; dy < bitmapDst.bmHeight; ++dy, pDestRow += bitmapDst.bmWidthBytes )
    {
        RGBQUAD* pRowPixel = (RGBQUAD*)pDestRow;
        for ( int dx = 0; dx < bitmapDst.bmWidth; ++dx, ++pRowPixel )
        {
            register long accsxx10000 = dx * 10000;
            register long accsyx10000 = dy * 10000;

            const SParticle* const* ppParticle = &m_visibleParticles[0];
            for ( ; *ppParticle != NULL; ++ppParticle )
            {
                const SParticle& particle = **ppParticle;

                long cx = dx - particle.x;
                long cy = dy - particle.y;
                long index = (cx + m_width) + (cy + m_height) * m_doubleWidth;

                long attnx100 = m_attenuationx100[index];
                
                long distCoefx100 = m_distanceCoefx100[index];
                long angle = distCoefx100 + particle.phaseShiftx100;
                long angleIndex = (angle % 36000) + 36000;

                long wavex1000000 = particle.amplitudex100 * attnx100 * m_cosinex100[angleIndex];

                long cosinex100 = m_cosineCoefx100[index];
                long sinex100 = m_sineCoefx100[index];
                long sxx10000 = (wavex1000000 * cosinex100) / 10000;
                long syx10000 = (wavex1000000 * sinex100) / 10000;

                accsxx10000 += sxx10000;
                accsyx10000 += syx10000;
            }    

            if ( accsxx10000 < 0 )
                accsxx10000 = 0;
            else if ( accsxx10000 > maxXx10000 )
                accsxx10000 = maxXx10000;
            if ( accsyx10000 < 0 )
                accsyx10000 = 0;
            else if ( accsyx10000 > maxYx10000 )
                accsyx10000 = maxYx10000;

            _SetColor(pRowPixel, bitmapSrc, accsxx10000, accsyx10000);
        }
    }
}

void CParticleSystem::Update()
{
    size_t visibleIndex = 0;
    const size_t countParticles = m_particles.size();
    for ( size_t i = 0; i < countParticles; ++i )
    {
        SParticle& particle = m_particles[i];

        // if particle was burn and its amplitude not so small yet
        // then make it visible, otherwise re-create particle
        if ( particle.amplitudex100 > AMPLITUDE_MINIMUM_x100 )
        {
            // make amplitude attenuation
            particle.amplitudex100 = (long)((double)particle.amplitudex100 * particle.attenuation);
            
            // make phase offset
            particle.phaseShiftx100 -= PHASE_SHIFT_DEG_x100; 

            m_visibleParticles[visibleIndex] = &particle;
            ++visibleIndex;
        }
        else
        {
            // re-init particle
            _InitParticle(particle);
        }
    }

    m_visibleParticles[visibleIndex] = NULL;
}

void CParticleSystem::_InitParticle(SParticle& particle) const
{
    particle.x = 30 + rand() % (m_width - 60);
    particle.y = 30 + rand() % (m_height - 60);
    particle.amplitudex100 = AMPLITUDE_MAXIMUM_x100;
    particle.phaseShiftx100 = 0;
    particle.attenuation = (double)(60 + rand()%40) / 100.0;
}

void CParticleSystem::_SetColor(RGBQUAD* pPixel, const CImage& image, long sxx10000, long syx10000)
{
    long x = (long)(sxx10000 / 10000);
    long y = (long)(syx10000 / 10000);
    long sxi = x * 10000;
    long syi = y * 10000;
    long frsx = sxx10000 - sxi;
    long frsy = syx10000 - syi;
    long frsx1 = 10000 - frsx;      
    long frsy1 = 10000 - frsy;
    long frsx_frsy = frsx * frsy / 100;
    long frsx1_frsy = frsx1 * frsy / 100;
    long frsx_frsy1 = frsx * frsy1 / 100;
    long frsx1_frsy1 = frsx1 * frsy1 / 100;
  
    const RGBTRIPLE* pixelRow0 = (RGBTRIPLE*)image.GetPixelAddress(x, y);
    const RGBTRIPLE* pixelRow1 = (RGBTRIPLE*)((BYTE*)pixelRow0 + image.GetPitch());

    const RGBTRIPLE* pixel = pixelRow0;
    const RGBTRIPLE* pixel_x = pixelRow0 + 1;
    const RGBTRIPLE* pixel_y = pixelRow1;
    const RGBTRIPLE* pixel_xy = pixelRow1 + 1;

    int r_color =  pixel->rgbtRed;
    int r_color_x = pixel_x->rgbtRed;
    int r_color_y = pixel_y->rgbtRed;
    int r_color_xy = pixel_xy->rgbtRed;
    int r_newColor = (int)( 
        r_color    * frsx1_frsy1 +
        r_color_x  * frsx_frsy1 +             
        r_color_y  * frsx1_frsy  +
        r_color_xy * frsx_frsy 
        );

    int g_color =  pixel->rgbtGreen;
    int g_color_x = pixel_x->rgbtGreen;
    int g_color_y = pixel_y->rgbtGreen;
    int g_color_xy = pixel_xy->rgbtGreen;
    int g_newColor = (int)( 
        g_color    * frsx1_frsy1 +
        g_color_x  * frsx_frsy1 +             
        g_color_y  * frsx1_frsy  +
        g_color_xy * frsx_frsy 
        );

    int b_color =  pixel->rgbtBlue;
    int b_color_x = pixel_x->rgbtBlue;
    int b_color_y = pixel_y->rgbtBlue;
    int b_color_xy = pixel_xy->rgbtBlue;
    int b_newColor = (int)( 
        b_color    * frsx1_frsy1 +
        b_color_x  * frsx_frsy1 +             
        b_color_y  * frsx1_frsy  +
        b_color_xy * frsx_frsy 
        );

    pPixel->rgbRed = r_newColor / 1000000;
    pPixel->rgbGreen = g_newColor / 1000000;
    pPixel->rgbBlue = b_newColor / 1000000;
}
