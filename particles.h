#pragma once

#include <vector>

class CParticleSystem
{
private:
    struct SParticle
    {
    public:
        long x; // X
        long y; // Y
        long amplitudex100; // amplitude to emulate attenuation at time
        long phaseShiftx100; // phase offset to emulate running wave, degrees
        double attenuation;
    };
public:
    CParticleSystem(size_t count, long width, long height);
    ~CParticleSystem();

    void Update();
    void Draw(const CImage& bitmapSrc, const BITMAP& bitmapDst) const;

private:
    inline void _InitParticle(SParticle& particle) const;
    inline static void _SetColor(RGBQUAD* pPixel, const CImage& image, long sxx10000, long syx10000);

private:
    const long m_width;
    const long m_doubleWidth;
    const long m_height;
    const long m_doubleHeight;
    std::vector<SParticle> m_particles;
    std::vector<SParticle*> m_visibleParticles;
    std::vector<long> m_distanceCoefx100;
    std::vector<long> m_attenuationx100;
    std::vector<long> m_cosineCoefx100;
    std::vector<long> m_sineCoefx100;
    std::vector<long> m_cosinex100;
};
