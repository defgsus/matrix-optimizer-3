/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/21/2015</p>
*/

#include <cmath>
#include <vector>

#include "DfDownsampler.h"


namespace MO {


struct DFDownsampler::Private
{
    Private(DFDownsampler * thread)
        : thread        (thread)
        , doStop        (false)
        , progress      (0.f)
        , threshold     (128)
    {

    }

    void downsample();

    DFDownsampler * thread;

    volatile bool doStop;
    volatile int progress;
    QImage inImage, outImage;
    QSize outRes, scanRange;
    uchar threshold;
};


DFDownsampler::DFDownsampler(QObject * parent)
    : QThread       (parent)
    , p_            (new Private(this))
{

}

DFDownsampler::~DFDownsampler()
{
    delete p_;
}

void DFDownsampler::setInputImage(const QImage & img)
{
    p_->inImage = img.convertToFormat(QImage::Format_Grayscale8);
}

void DFDownsampler::setOutputResolution(const QSize & s)
{
    p_->outRes = s;
}

void DFDownsampler::setSamplingRange(const QSize & s)
{
    p_->scanRange = s;
}

void DFDownsampler::setThreshold(uchar t)
{
    p_->threshold = t;
}

const QImage& DFDownsampler::getOutputImage() const
{
    return p_->outImage;
}

int DFDownsampler::progress() const
{
    return p_->progress;
}

void DFDownsampler::stop()
{
    p_->doStop = true;
    wait();
    p_->progress = 0.f;
    emit progressChanged(0.f);
}

void DFDownsampler::run()
{
    p_->downsample();
}

void DFDownsampler::Private::downsample()
{
    doStop = false;
    progress = 0.f;
    emit thread->progressChanged(progress);

    outImage = QImage(outRes, QImage::Format_Grayscale8);

    // maximum possible distance for given scanRange
    float maxDist = .5f * std::sqrt(float(scanRange.width() * scanRange.width()
                                  + scanRange.height() * scanRange.height()));
    // prepare float buffer
    std::vector<float> buf(outRes.width() * outRes.height());
    for (auto & f : buf)
        f = maxDist;

    // go through output image
    for (int oy = 0; oy < outImage.height(); ++oy)
    {
        const int prog = float(oy) / outImage.height() * 100.;
        if (prog != progress)
        {
            progress = prog;
            emit thread->progressChanged(progress);
        }

        // get corresponding input pixel
        const int iy = oy * inImage.height() / outImage.height();

        for (int ox = 0; ox < outImage.width(); ++ox)
        {
            const int ix = ox * inImage.width() / outImage.width();

            // starting inside or outside of graphic?
            const bool isInside = qRed(inImage.pixel(ix, iy)) >= threshold;

            // scan for min distance to all edges
            float minD = maxDist;
            for (int sy = 0; sy < scanRange.height(); ++sy)
            {
                if (doStop)
                    return;

                const int Y = iy + sy - scanRange.height() / 2;
                if (Y < 0 || Y >= inImage.height())
                    continue;

                for (int sx = 0; sx < scanRange.width(); ++sx)
                {
                    const int X = ix + sx - scanRange.width() / 2;
                    if (X < 0 || X >= inImage.width())
                        continue;

                    // is this pixel inside or outside?
                    const bool isPix = qRed(inImage.pixel(X, Y)) >= threshold;

                    // crossed edge?
                    if (isPix != isInside)
                    {
                        // distance to pixel
                        float d = std::sqrt(float((X-ix)*(X-ix) + (Y-iy)*(Y-iy)));

                        minD = std::min(minD, d);
                    }

                } // sx
            } // sy

            // negative distance when started inside
            if (isInside)
                minD = -minD;

            // store result
            buf[oy * outImage.width() + ox] = minD;

        } // ox
    } // oy


    // convert to image
    for (int oy = 0; oy < outImage.height(); ++oy)
    for (int ox = 0; ox < outImage.width(); ++ox)
    {
        float f = buf[oy * outImage.width() + ox];
        outImage.scanLine(oy)[ox]
        // normalize
                = uchar( f / maxDist * 127 + 128 );
    }
}


} // namespace MO
