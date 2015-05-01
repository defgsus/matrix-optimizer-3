/** @file irmap.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 29.04.2015</p>
*/

#include <sndfile.h>

#include <QObject> // for tr()
#include <QPainter>

#include "irmap.h"
#include "gui/painter/grid.h"
#include "gui/painter/valuecurve.h"
#include "math/functions.h"
#include "math/constants.h"
#include "io/error.h"

//#define MO_IRMAP_TL

namespace MO {
namespace AUDIO {

namespace {

    class TimelineCurveData : public GUI::PAINTER::ValueCurveData
    {
    public:
        MATH::Timeline1D * timeline;
        virtual Double value(Double time) const { return timeline->get(time); }
    };

}

const Float IrMap::p_convert_ = 1000.f;

IrMap::IrMap()
{
    clear();
}

bool IrMap::isEmpty() const
{
#ifdef MO_IRMAP_TL
    return p_tl_.empty();
#else
    return p_map_.empty();
#endif
}

size_t IrMap::numSamples() const
{
#ifdef MO_IRMAP_TL
    return p_tl_.size();
#else
    return p_map_.size();
#endif
}

QString IrMap::getInfo() const
{
    return QObject::tr("samples %1; amp %2 - %3; length %4 - %5")
                   .arg(numSamples())
                   .arg(p_min_amp_)
                   .arg(p_max_amp_)
                   .arg(p_min_dist_)
                   .arg(p_max_dist_);
}

QImage IrMap::getImage(const QSize &res)
{
    QImage img(res, QImage::Format_ARGB32);
    img.fill(Qt::black);


    if (isEmpty())
        return img;

    QPainter p(&img);

    p.setRenderHint(QPainter::Antialiasing, true);

    GUI::UTIL::ViewSpace vs(0., -p_max_amp_, p_max_dist_, p_max_amp_ * 2.);
    GUI::PAINTER::Grid grid;
    grid.setViewSpace(vs);
    grid.paint(p);

#ifdef MO_IRMAP_TL

    GUI::PAINTER::ValueCurve curve;
    TimelineCurveData cdata;
    cdata.timeline = &p_tl_;
    curve.setCurveData(&cdata);
    vs.setScaleX(p_max_dist_ * p_convert_);
    curve.setViewSpace(vs);
    curve.setOverpaint(3);
    curve.paint(p);

#else

    auto samples = getSamples(48000);

    p.setPen(QPen(QColor(55,255,55,50)));

    const Float
            invDist = Float(img.width()) / std::max(size_t(1), samples.size()),
            invAmp = -.5f * img.height() / std::max(0.000001f, p_max_amp_);

    // draw a line per sample
    for (size_t i=0; i<samples.size(); ++i)
    {
        QPointF p0(invDist * i, .5 * img.height()),
                p1(p0.x(), p0.y() + samples[i] * invAmp);
        p.drawLine(p0, p1);
    }
    /*
    for (auto it = p_map_.begin(); it != p_map_.end(); ++it)
    {
        QPointF p0(it->first * invDist, .5 * img.height()),
                p1(p0.x(), p0.y() + it->second * invAmp);
        p.drawLine(p0, p1);
    }*/
#endif

    p.end();
    return img;
}


void IrMap::clear()
{
#ifdef MO_IRMAP_TL
    p_tl_.clear();
#else
    p_map_.clear();
#endif
    p_max_amp_ = p_min_amp_ = p_max_dist_ = p_min_dist_ = 0.f;
}

void IrMap::addSample(Float distance, Float amplitude)
{
    if (isEmpty())
    {
        p_max_amp_ = p_min_amp_ = std::abs(amplitude);
        p_max_dist_ = p_min_dist_ = distance;

#ifdef MO_IRMAP_TL
        p_tl_.add(0., 0., MATH::Timeline1D::Point::SPLINE6);
        p_tl_.add(distance * p_convert_, amplitude, MATH::Timeline1D::Point::SPLINE6);
#else
        p_map_.insert(std::make_pair(distance, amplitude));
#endif
        return;
    }

#ifndef MO_IRMAP_TL
    if (p_map_.find(distance) == p_map_.end())
#endif
    {
        p_min_amp_ = std::min(p_min_amp_, std::abs(amplitude));
        p_max_amp_ = std::max(p_max_amp_, std::abs(amplitude));
        p_min_dist_ = std::min(p_min_dist_, distance);
        p_max_dist_ = std::max(p_max_dist_, distance);

#ifdef MO_IRMAP_TL
        p_tl_.add(distance * p_convert_, amplitude, MATH::Timeline1D::Point::SPLINE6);
#else
        p_map_.insert(std::make_pair(distance, amplitude));
#endif
    }
}

std::vector<F32> IrMap::getSamples(Float sr, Float sos)
{
    int len = std::max(0.f, p_max_dist_) / sos * sr;

    std::vector<F32> samples;
    if (!len)
        return samples;

#ifdef MO_IRMAP_TL
    samples.reserve(len);

    const Float invAmp = .99 / std::max(0.000001f, p_max_amp_);

    for (int i=0; i<len; ++i)
    {
        Float t = Float(i) / len * p_max_dist_ * p_convert_;
        samples.push_back(p_tl_.get(t)// * invAmp
                                    );
    }

#else

#if 0 // no interpolation
    samples.resize(len);

    const Float invDist = Float(len) / std::max(0.000001f, p_max_dist_);

    for (auto it = p_map_.begin(); it != p_map_.end(); ++it)
    {
        const Float pos = it->first * invDist,
                    amp = it->second;

        const int ipos = pos;
        if (ipos < 0 || ipos >= len)
            continue;

        samples[ipos] = amp;
    }
#elif 0 // averaging

    samples.resize(len, 0.f);
    std::vector<Float> counts;
    counts.resize(len, 0);

    const Float invDist = Float(len) / std::max(0.000001f, p_max_dist_);

    for (auto it = p_map_.begin(); it != p_map_.end(); ++it)
    {
        const Float pos = it->first * invDist,
                    amp = it->second;

        const int ipos = pos;
        if (ipos < 0 || ipos >= len)
            continue;

        Float f = MATH::fract(pos), f1 = 1.f - f;

        samples[ipos] += amp * f1;
        counts[ipos] += f1;
        if (ipos + 1 >= len)
            continue;
        samples[ipos+1] += amp * f;
        counts[ipos+1] += f;
    }

    for (size_t i=0; i<samples.size(); ++i)
        if (counts[i])
            samples[i] /= counts[i];


#elif 0 // bell averaging (precalc)

    samples.resize(len, 0.f);
    std::vector<Float> counts;
    counts.resize(len, 0);

    if (p_impulse_.empty())
    {
        size_t num = 100;
        for (size_t i=0; i<num; ++i)
        {
            Float t = Float(i) / (num-1);
            //p_impulse_.push_back(std::pow(std::sin(t * PI), 2.));
            //p_impulse_.push_back( std::sin(t * TWO_PI) );
        }
    }

    const Float invDist = Float(len) / std::max(0.000001f, p_max_dist_);

    for (auto it = p_map_.begin(); it != p_map_.end(); ++it)
    {
        const Float pos = it->first * invDist,
                    amp = it->second;

        Float f = MATH::fract(pos), f1 = 1.f - f;

        const int ipos = pos;
        for (size_t j=0; j<p_impulse_.size(); ++j)
        {
            int k = ipos + j;
            if (k < 0 || k >= len)
                continue;

            samples[k] += amp * f1 * p_impulse_[j];
            counts[k] += f1;// * p_impulse_[j];
            ++k;
            if (k < len)
            {
                samples[k] += amp * f * p_impulse_[j];
                counts[k] += f;// * p_impulse_[j];
            }
        }
    }

    for (size_t i=0; i<samples.size(); ++i)
        if (counts[i])
            samples[i] /= std::max(1.f, counts[i]);

#elif 0 // bell averaging (dynamic)

    samples.resize(len, 0.f);
    std::vector<Float> counts;
    counts.resize(len, 0);

    const Float invDist = Float(len) / std::max(0.000001f, p_max_dist_);

    for (auto it = p_map_.begin(); it != p_map_.end(); ++it)
    {
        const Float pos = it->first * invDist,
                    amp = it->second;

        Float f = MATH::fract(pos), f1 = 1.f - f;

        size_t num = 3 + it->first / 6;

        const int ipos = pos;
        for (size_t j=0; j<num; ++j)
        {
            int k = ipos + j;
            if (k < 0 || k >= len)
                continue;

            Float t = Float(j) / (num-1);
            Float imp = std::pow(std::sin(t * PI), 2.);


            samples[k] += amp * f1 * imp;
            counts[k] += f1;// * p_impulse_[j];
            ++k;
            if (k < len)
            {
                samples[k] += amp * f * imp;
                counts[k] += f;// * p_impulse_[j];
            }
        }
    }

    for (size_t i=0; i<samples.size(); ++i)
        if (counts[i])
            samples[i] /= std::max(1.f, counts[i]);

#elif 1 // bell averaging (dynamic) normalized

    samples.resize(len, 0.f);

    const Float invDist = Float(len) / std::max(0.000001f, p_max_dist_);

    for (auto it = p_map_.begin(); it != p_map_.end(); ++it)
    {
        const Float pos = it->first * invDist,
                    amp = it->second;

        Float f = MATH::fract(pos), f1 = 1.f - f;

        size_t num = 3 + it->first / 6;

        const int ipos = pos;
        for (size_t j=0; j<num; ++j)
        {
            int k = ipos + j;
            if (k < 0 || k >= len)
                continue;

            Float t = Float(j) / (num-1);
            Float imp = std::pow(std::sin(t * PI), 2.);


            samples[k] += amp * f1 * imp;
            ++k;
            if (k < len)
            {
                samples[k] += amp * f * imp;
            }
        }
    }

    Float ma = 0.000001;
    for (size_t i=0; i<samples.size(); ++i)
        ma = std::max(samples[i], ma);
    for (size_t i=0; i<samples.size(); ++i)
        samples[i] /= ma;

#elif 1 // max
    samples.resize(len);

    const Float invDist = Float(len) / std::max(0.000001f, p_max_dist_);

    for (auto it = p_map_.begin(); it != p_map_.end(); ++it)
    {
        const Float pos = it->first * invDist,
                    amp = it->second;

        const int ipos = pos;
        if (ipos < 0 || ipos >= len)
            continue;

        samples[ipos] = std::max(amp, samples[ipos]);
    }
#endif

#endif

    return samples;
}

void IrMap::saveWav(const QString &filename, Float sample_rate, Float speed_of_sound)
{
    auto samples = getSamples(sample_rate, speed_of_sound);

    SF_INFO info;
    info.channels = 1;
    info.samplerate = sample_rate;
    info.frames = samples.size();
    info.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

    SNDFILE *f = sf_open(filename.toStdString().c_str(), SFM_WRITE, &info);
    if (!f)
        MO_IO_ERROR(WRITE, "could not open file for writing audio '" << filename << "'\n"
                    << sf_strerror((SNDFILE*)0));


    uint e = sf_writef_float(f, (const F32*)&samples[0], samples.size());
    sf_close(f);

    if (e != samples.size())
    {
        MO_IO_ERROR(WRITE, "could not write all of soundfile '" << filename << "'\n"
                    "expected " << samples.size() << " frames, got " << e );
    }
}

} // namespace AUDIO
} // namespace MO
