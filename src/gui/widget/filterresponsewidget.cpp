/** @file filterresponsewidget.cpp

    @brief MultiFilter response display

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#include <QPainter>

#include "filterresponsewidget.h"
#include "audio/tool/multifilter.h"
#include "math/constants.h"
#include "io/log.h"

namespace MO {
namespace GUI {

FilterResponseWidget::FilterResponseWidget(QWidget *parent)
    : QWidget       (parent),
      filter_       (new AUDIO::MultiFilter()),
      response_     (128),
      sampleRate_   (44100),
      lowFreq_      (20),
      logScale_     (false)
{
    setObjectName("_FilterResponseWidget");

    setMinimumSize(100,100);

    brushBack_ = QBrush(QColor(0,0,0));
    penGrid_ = QPen(QColor(60,60,60));
    penFreq_ = QPen(QColor(60,140,60));
    penFreq_.setStyle(Qt::DotLine);
    penCurve_ = QPen(QColor(100,200,100));
}

FilterResponseWidget::~FilterResponseWidget()
{
    delete filter_;
}

void FilterResponseWidget::setFilter(const AUDIO::MultiFilter & f)
{
    *filter_ = f;
    filter_->setSampleRate(sampleRate_);
    calcResponse_();
    update();
}

void FilterResponseWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    // background
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setBrush(brushBack_);
    p.setPen(Qt::NoPen);
    p.drawRect(rect());

    // grid
    p.setBrush(Qt::NoBrush);
    p.setPen(penGrid_);

    // x (freq)
    F32 freq = 0;
    while (freq < sampleRate_)
    {
        int x = XForFreq(freq);
        p.drawLine(x,0, x,height());
        freq += 2000;
    }

    // y (amp)
    int num = 4;
    for (int i=0; i<num; ++i)
    {
        int y = height() - 1 - i * height() / num;
        p.drawLine(0,y, width(),y);
    }

    // center freq
    p.setPen(penFreq_);
    int x = XForFreq(filter_->frequency());
    p.drawLine(x,0, x,height());

    // response curve
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(penCurve_);

    int lx=0, ly=0;
    for (uint i=0; i<response_.size(); ++i)
    {
        int x = XForFreq(freqForBand(i));
        int y = height() - 1 - height() * response_[i] / 4;

        if (i>0)
            p.drawLine(lx,ly, x,y);

        lx = x;
        ly = y;
    }
}

int FilterResponseWidget::XForFreq(F32 freq) const
{
    return freq / (sampleRate_ / 2) * width();
}

F32 FilterResponseWidget::freqForX(int x) const
{
    return F32(x) / width() * sampleRate_ / 2;
}

F32 FilterResponseWidget::freqForBand(uint band) const
{
    return lowFreq_ + F32(band) / response_.size() * (sampleRate_ - lowFreq_) / 2;
}

void FilterResponseWidget::calcResponse_()
{
    MO_DEBUG_GUI("FilterResponseWidget::calcResponse_() t="
             << filter_->typeName() << " f=" << filter_->frequency()
             << " r=" << filter_->resonance() << " o=" << filter_->order());

    std::vector<F32>
            input(2000),
            output(input.size());

    for (uint j=0; j<response_.size(); ++j)
    {
        const F32 freq = freqForBand(j);

        // create a sine tone
        for (uint i=0; i<input.size(); ++i)
            input[i] = std::sin(F32(i) / sampleRate_ * TWO_PI * freq);

        // filter it
        filter_->reset();
        filter_->process(&input[0], &output[0], input.size());

        // get amplitude
        F32 amp = std::abs(output[0]);
        for (uint i=1; i<output.size(); ++i)
            amp = std::max(amp, std::abs(output[i]));

        response_[j] = amp;
    }
}


} // namespace GUI
} // namespace MO
