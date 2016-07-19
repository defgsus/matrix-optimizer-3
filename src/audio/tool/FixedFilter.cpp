/** @file fixedfilter.cpp

    @brief Butterworth/Chebychev/Bessel filter with coefficient generator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 27.09.2014</p>

    <p>adapted from: Dr Anthony J. Fisher / http://www-users.cs.york.ac.uk/~fisher/software/mkfilter</p>
*/

#include <complex>

#include "FixedFilter.h"
#include "io/error.h"
#include "math/constants.h"
#include "io/log.h"

namespace MO {
namespace AUDIO {

namespace
{

typedef std::complex<Double> Complex;

static const Complex bessel_poles[] =
  { /* table produced by /usr/fisher/bessel --	N.B. only one member of each C.Conj. pair is listed */
    { -1.000000e+00,  0.000000e+00 },	 { -8.660254e-01, -5.000000e-01 },    { -9.416000e-01,	0.000000e+00 },
    { -7.456404e-01, -7.113666e-01 },	 { -9.047588e-01, -2.709187e-01 },    { -6.572112e-01, -8.301614e-01 },
    { -9.264421e-01,  0.000000e+00 },	 { -8.515536e-01, -4.427175e-01 },    { -5.905759e-01, -9.072068e-01 },
    { -9.093907e-01, -1.856964e-01 },	 { -7.996542e-01, -5.621717e-01 },    { -5.385527e-01, -9.616877e-01 },
    { -9.194872e-01,  0.000000e+00 },	 { -8.800029e-01, -3.216653e-01 },    { -7.527355e-01, -6.504696e-01 },
    { -4.966917e-01, -1.002509e+00 },	 { -9.096832e-01, -1.412438e-01 },    { -8.473251e-01, -4.259018e-01 },
    { -7.111382e-01, -7.186517e-01 },	 { -4.621740e-01, -1.034389e+00 },    { -9.154958e-01,	0.000000e+00 },
    { -8.911217e-01, -2.526581e-01 },	 { -8.148021e-01, -5.085816e-01 },    { -6.743623e-01, -7.730546e-01 },
    { -4.331416e-01, -1.060074e+00 },	 { -9.091347e-01, -1.139583e-01 },    { -8.688460e-01, -3.430008e-01 },
    { -7.837694e-01, -5.759148e-01 },	 { -6.417514e-01, -8.175836e-01 },    { -4.083221e-01, -1.081275e+00 },
  };

static const Complex cmone ( -1.0, 0.0 );
static const Complex czero (  0.0, 0.0 );
static const Complex cone  (  1.0, 0.0 );
static const Complex ctwo  (  2.0, 0.0 );
static const Complex chalf (  0.5, 0.0 );


} // namespace



// ----------------------------------- FixedFilter::Private ----------------------------------


class FixedFilter::Private
{
public:

    Private();

    void calcCoefficients()
    {
        setdefaults();
        compute_s();
        normalize();
        compute_z();
        expandpoly();
        getGain();

        //dumpCoeffs();
    }

    template <typename F>
    void process(const F * input, uint inputStride,
                       F * output, uint outputStride, uint size, F amp);

    void dumpCoeffs();

    // ---- coefficient calculation ----

    void setdefaults();
    /** evaluate response, substituting for z */
    Complex evaluate(std::vector<Complex> & topco,
                     std::vector<Complex> & botco,
                     uint np, const Complex & z);
    /** evaluate polynomial in z, substituting for z */
    Complex eval(std::vector<Complex> &coeffs, uint np, const Complex & z);
    /** compute S-plane poles for prototype LP filter */
    void compute_s();
    void choosepole(const Complex& z);
    void normalize();
    /** given S-plane poles, compute Z-plane poles */
    void compute_z();
    /** given Z-plane poles & zeros, compute top & bot polynomials in Z,
        and then recurrence relation */
    void expandpoly();
    /** compute product of poles or zeros as a polynomial of z */
    void expand(std::vector<Complex> & pz, std::vector<Complex> & coeffs);
    /** multiply factor (z-w) into coeffs */
    void multin(const Complex & w, std::vector<Complex> & coeffs);
    void getGain();

    FixedFilter::BandType bandType;
    FixedFilter::FilterType type;
    uint sr;
    Double freq, freqRange;

    uint order, numpoles;
    Double raw_alpha1, raw_alpha2;
    Complex dc_gain, fc_gain, hf_gain;

    Double warped_alpha1, warped_alpha2, chebrip;
    uint polemask;

    bool specifiedPolesOnly,
         noPrewarp;

    std::vector<Complex>
        spoles, zpoles, zzeros;
    std::vector<Double>
        xcoeffs, ycoeffs,
    // execution buffers
        xv, yv;

    Double maxgain, clip;
};

FixedFilter::Private::Private()
    : bandType      (BT_LOWPASS),
      type          (FT_BUTTERWORTH),
      sr            (44100),
      freq          (1000),
      freqRange     (100),
      order         (2),
      chebrip       (-1),
      specifiedPolesOnly(false),
      noPrewarp     (false),
      clip          (4)
{

}


void FixedFilter::Private::setdefaults()
{
    /* expected values:
        chebrip
        raw_alpha1
        raw_alpha2
        order
        specifiedPolesOnly==false or polemask
        noPrewarp
        type
        bandType
    */

    const uint poles = order * 2;

    spoles.resize(poles);
    zpoles.resize(poles);
    zzeros.resize(poles);
    xcoeffs.resize(poles+1);
    ycoeffs.resize(poles+1);
    xv.resize(poles+1);
    yv.resize(poles+1);

    if (!specifiedPolesOnly)
        polemask = 0xffffffff;

    if (bandType == BT_BANDPASS)
    {
        raw_alpha1 = std::max(1.0,std::min(Double(sr)/2-1, (freq - freqRange * 0.5))) / sr;
        raw_alpha2 = std::max(1.0,std::min(Double(sr)/2-1, (freq + freqRange * 0.5))) / sr;
    }
    else
        raw_alpha1 = raw_alpha2 = freq / sr;
}

Complex FixedFilter::Private::evaluate(
        std::vector<Complex> & topco,
        std::vector<Complex> & botco,
        uint np, const Complex & z)
{
    return eval(topco, np, z) / eval(botco, np, z);
}

Complex FixedFilter::Private::eval(std::vector<Complex> &coeffs, uint np, const Complex & z)
{
    Complex sum = czero;

    for (int i=np; i >= 0; i--)
        sum = (sum * z) + coeffs[i];

    return sum;
}


void FixedFilter::Private::compute_s()
{
    numpoles = 0;
    // Bessel filter
    if (type == FT_BESSEL)
    {
        int p = (order*order)/4; /* ptr into table */
        if (order & 1)
            choosepole(bessel_poles[p++]);
        for (uint i=0; i < order/2; ++i)
        {
            choosepole(bessel_poles[p]);
            choosepole(std::conj(bessel_poles[p]));
            p++;
        }
    }

    // Butterworth filter
    if (type == FT_BUTTERWORTH || type == FT_CHEBYCHEV)
    {
        for (uint i=0; i < 2*order; ++i)
        {
            Complex s(0.0,
                      (order & 1) ? Double(i * PI) / order : ((i + 0.5) * PI) / order);
            choosepole(std::exp(s));
        }
    }

    // modify for Chebyshev (p. 136 DeFatta et al.)
    if (type == FT_CHEBYCHEV)
    {
        Double rip, eps, y;
        if (chebrip >= 0.0)
        {
            MO_WARNING("FixedFilter: Cbebyshev ripple is " << chebrip << " dB; must be < 0.0");
            chebrip = -0.1;
        }
        rip = std::pow(10.0, -chebrip / 10.0);
        eps = std::sqrt(rip - 1.0);
        y = std::asinh(1.0 / eps) / (Double)order;
        if (y <= 0.0)
        {
            MO_WARNING("FixedFilter: bug: Chebyshev y=" << y << "; must be > 0.0");
            y = 0.1; // ???
        }
        for (uint i=0; i < numpoles; ++i)
        {
            spoles[i] = Complex(
                            spoles[i].real() * std::sinh(y),
                            spoles[i].imag() * std::cosh(y));
        }
    }
}

void FixedFilter::Private::choosepole(const Complex& z)
{
    if (z.real() < 0.0)
    {
        if (polemask & 1)
            spoles[numpoles++] = z;
        polemask >>= 1;
    }
}

void FixedFilter::Private::normalize()
{
    // for bilinear transform, perform pre-warp on alpha values
    if (noPrewarp)
    {
        warped_alpha1 = raw_alpha1;
        warped_alpha2 = raw_alpha2;
    }
    else
    {
        warped_alpha1 = std::tan(PI * raw_alpha1) / PI;
        warped_alpha2 = std::tan(PI * raw_alpha2) / PI;
    }

    Complex w1(TWO_PI * warped_alpha1, 0.0),
            w2(TWO_PI * warped_alpha2, 0.0);

    /* transform prototype into appropriate filter type (lp/hp/bp) */
    switch (bandType)
    {
        case BT_LOWPASS:
            for (uint i=0; i < numpoles; i++)
                spoles[i] = spoles[i] * w1;
        break;

        case BT_HIGHPASS:
            for (uint i=0; i < numpoles; i++)
                spoles[i] = w1 / spoles[i];
            /* also N zeros at (0,0) */
        break;

        case BT_BANDPASS:
        {
            Complex w0 = std::sqrt(w1 * w2),
                    bw = w2 - w1;
            for (uint i=0; i < numpoles; i++)
            {
                Complex hba = chalf * (spoles[i] * bw),
                        temp = w0 / hba;
                temp = std::sqrt(cone - (temp * temp));
                spoles[i] = hba * (cone + temp);
                spoles[numpoles+i] = hba * (cone - temp);
            }
            /* also N zeros at (0,0) */
            numpoles *= 2;
        break;
        }
    }
}

void FixedFilter::Private::compute_z()
{
    for (uint i=0; i < numpoles; i++)
    {
        /* use bilinear transform */
        Complex top = ctwo + spoles[i],
                bot = ctwo - spoles[i];
        zpoles[i] = top / bot;

        switch (bandType)
        {
            case BT_LOWPASS:  zzeros[i] = cmone; break;
            case BT_HIGHPASS: zzeros[i] = cone; break;
            case BT_BANDPASS: zzeros[i] = (i & 1) ? cone : cmone; break;
        }
    }
}

void FixedFilter::Private::expandpoly()
{
    std::vector<Complex>
            topcoeffs(numpoles+1),
            botcoeffs(numpoles+1);

    expand(zzeros, topcoeffs);
    expand(zpoles, botcoeffs);
    dc_gain = evaluate(topcoeffs, botcoeffs, numpoles, cone);
    Complex st(0.0, TWO_PI * 0.5 * (raw_alpha1 + raw_alpha2)), /* "jwT" for centre freq. */
            zfc = std::exp(st);
    fc_gain = evaluate(topcoeffs, botcoeffs, numpoles, zfc);
    hf_gain = evaluate(topcoeffs, botcoeffs, numpoles, cmone);
    for (uint i=0; i <= numpoles; ++i)
    {
        xcoeffs[i] =   topcoeffs[i].real() / botcoeffs[numpoles].real();
        ycoeffs[i] = -(botcoeffs[i].real() / botcoeffs[numpoles].real());
    }
}

void FixedFilter::Private::expand(std::vector<Complex> & pz, std::vector<Complex> & coeffs)
{
    coeffs[0] = cone;
    for (uint i=0; i < numpoles; i++)
        coeffs[i+1] = czero;
    for (uint i=0; i < numpoles; i++)
        multin(pz[i], coeffs);

    /* check computed coeffs of z^k are all real */
    for (uint i=0; i < numpoles+1; i++)
    {
        if (std::abs(coeffs[i].imag()) > 1.0e-10)
        {
            MO_WARNING("FixedFilter: coeff of z^" << i << " is not real; "
                       "poles are not complex conjugates");
            // XXX invalidate filter
        }
    }
}

void FixedFilter::Private::multin(const Complex& w, std::vector<Complex> & coeffs)
{
    Complex nw = -w;

    for (uint i=numpoles; i >= 1; i--)
      coeffs[i] = (nw * coeffs[i]) + coeffs[i-1];

    coeffs[0] = nw * coeffs[0];
}

namespace
{
    std::string gainStr(const std::string& str, const Complex& c)
    {
        std::stringstream s;

        Double r = std::hypot(c.imag(), c.real());
        s << "gain at " << str << ":   mag = " << r;
        if (r > 1.0e-10)
            s << "   phase = " << (std::atan2(c.imag(), c.real()) / PI) << " pi";

        return s.str();
    }
}

void FixedFilter::Private::getGain()
{
    switch (bandType)
    {
        case BT_LOWPASS: maxgain = std::max(Double(0.00001),
                                std::hypot(dc_gain.imag(), dc_gain.real())); break;
        case BT_HIGHPASS: maxgain = std::max(Double(0.00001),
                                std::hypot(hf_gain.imag(), hf_gain.real())); break;
        case BT_BANDPASS: maxgain = std::max(Double(0.00001),
                                std::hypot(fc_gain.imag(), fc_gain.real())); break;
    }
}

void FixedFilter::Private::dumpCoeffs()
{
    std::stringstream s;

    s << "FixedFilter: numpoles = " << numpoles << "\n";

    s << "raw alpha1    = " << raw_alpha1 <<
         "\nwarped alpha1 = " << warped_alpha1 <<
         "\nraw alpha2    = " << raw_alpha2 <<
         "\nwarped alpha2 = " << warped_alpha2 <<
         "\n" << gainStr("dc    ", dc_gain) <<
         "\n" << gainStr("centre", fc_gain) <<
         "\n" << gainStr("hf    ", hf_gain) << "\n";

    s << "\nS-plane zeros:\n";
    switch (bandType)
    {
        case BT_LOWPASS:
            s << "none\n";
        break;

        case BT_HIGHPASS:
            s << "\t" << czero << "\t" << numpoles << " times\n";
        break;

        case BT_BANDPASS:
            s << "\t" << czero << "\t" << (numpoles/2) << " times\n";
        break;
    }

    s << "\nS-plane poles:\n";
    for (uint i=0; i < numpoles; i++)
        s << "\t" << spoles[i] << "\n";

    s << "\nZ-plane zeros:\n";
    switch (bandType)
    {
        case BT_LOWPASS:
            s << "\t" << cmone << "\t" << numpoles << " times\n";
        break;

        case BT_HIGHPASS:
            s << "\t" << cone << "\t" << numpoles << " times\n";
        break;

        case BT_BANDPASS:
            s << "\t" << cone << "\t" << (numpoles/2) << " times\n";
            s << "\t" << cmone << "\t" << (numpoles/2) << " times\n";
        break;
    }

    s << "\nZ-plane poles:\n";
    for (uint i=0; i < numpoles; i++)
    {
        s << "\t" << zpoles[i] << "\n";
    }

    s << "\nRecurrence relation:\n"
      << "y[n] = ";
    for (uint i=0; i < numpoles+1; i++)
    {
        if (i > 0)
            s << "     + ";
        s << "(" << xcoeffs[i] << " * x[n-" << (numpoles-i) << "])\n";
    }
    s << "\n";
    for (uint i=0; i < numpoles; i++)
    {
        s << "     + (" << ycoeffs[i] << " * y[n-" << (numpoles-i) << "])\n";
    }

    MO_PRINT(s.str());
}



template <typename F>
void FixedFilter::Private::process(const F *input, uint inputStride,
                                         F *output, uint outputStride, uint size, F amp)
{
#define MO__CLIP(v__) std::max(-clip_,std::min(clip_, (v__) ))

    amp /= maxgain;

    for (uint i=0; i<size; ++i, input += inputStride, output += outputStride)
    {
        // shift buffer
        for (uint j=0; j < numpoles; j++)
        {
            xv[j] = xv[j+1];
            yv[j] = yv[j+1];
        }

        xv[numpoles] = yv[numpoles] = *input;

        // execute
        for (uint j=0; j < numpoles; j++)
            yv[numpoles] += (xcoeffs[j] * xv[j]) + (ycoeffs[j] * yv[j]);

        *output = yv[numpoles] * amp;
    }

#undef MO__CLIP
}


// ----------------------------------- FixedFilter ----------------------------------


FixedFilter::FixedFilter()
    : p_    (new Private())

{
    updateCoefficients();
    reset();
}

FixedFilter::~FixedFilter()
{
    delete p_;
}

uint FixedFilter::sampleRate() const { return p_->sr; }
uint FixedFilter::order() const { return p_->order; }
Double FixedFilter::frequency() const { return p_->freq; }
Double FixedFilter::bandpassSize() const { return p_->freqRange; }
FixedFilter::FilterType FixedFilter::type() const { return p_->type; }
FixedFilter::BandType FixedFilter::bandType() const { return p_->bandType; }
Double FixedFilter::chebychevRipple() const { return p_->chebrip; }
Double FixedFilter::clipping() const { return p_->clip; }

void FixedFilter::setSampleRate(uint sr) { p_->sr = std::max(uint(1), sr); }
void FixedFilter::setOrder(uint order) { p_->order = std::max(uint(1), std::min(uint(10), order )); }
void FixedFilter::setFrequency(Double freq) { p_->freq = std::max(Double(0.001), freq); }
void FixedFilter::setBandpassSize(Double freq) { p_->freqRange = std::max(Double(0.00001), freq); }
void FixedFilter::setType(FilterType type) { p_->type = type; }
void FixedFilter::setBandType(BandType type) { p_->bandType = type; }
void FixedFilter::setChebychevRipple(Double db) { p_->chebrip = db; }
void FixedFilter::setClipping(Double clip) { p_->clip = clip; }

void FixedFilter::reset()
{
    for (auto & v : p_->xv)
        v = 0.0;
    for (auto & v : p_->yv)
        v = 0.0;
}

void FixedFilter::updateCoefficients()
{
    MO_ASSERT(p_->sr > 0, "samplerate f***ed up");

    p_->calcCoefficients();
}

void FixedFilter::process(const F32 *input, uint inputStride,
                                F32 *output, uint outputStride, uint blockSize, F32 amp)
{
    p_->process(input, inputStride, output, outputStride, blockSize, amp);
}

void FixedFilter::process(const Double *input, uint inputStride,
                                Double *output, uint outputStride, uint blockSize, Double amp)
{
    p_->process(input, inputStride, output, outputStride, blockSize, amp);
}

} // namespace AUDIO
} // namespace MO
