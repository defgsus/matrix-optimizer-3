/** @file filterresponsewidget.cpp

    @brief MultiFilter response display

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#include <QPainter>
#include <QMouseEvent>
#include <QThread>
#include <cmath>

#include "filterresponsewidget.h"
#include "audio/tool/multifilter.h"
#include "math/constants.h"
#include "math/fft.h"
#include "io/log.h"

namespace MO {
namespace GUI {

namespace
{
    // thread for calculating the filter response
    class ResponseCalc : public QThread
    {
    public:

        ResponseCalc(FilterResponseWidget * w, std::vector<F32>& response)
            : QThread(w),
              w(w), response(response), doStop(false)
        { }

        virtual void run() Q_DECL_OVERRIDE;

        void calc(uint from, uint to);
        void calcFft();
        void stop();

    private:

        void calcResponse_();
        void calcFft_();

        FilterResponseWidget * w;
        std::vector<F32>& response;
        uint from, to;
        bool doFft_;
        volatile bool doStop;

        AUDIO::MultiFilter filter;
    };
}

FilterResponseWidget::FilterResponseWidget(QWidget *parent)
    : QWidget       (parent),
      filter_       (new AUDIO::MultiFilter()),
      sampleRate_   (44100),
      numBands_     (128),
      bufferSize_   (1024),
      lowFreq_      (20),
      logScale_     (false),
      doFft_        (false),
      thread_       (0)
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
    if (thread_ && thread_->isRunning())
        ((ResponseCalc*)thread_)->stop();

    delete filter_;
}

void FilterResponseWidget::setFilter(const AUDIO::MultiFilter & f, bool upd)
{
    *filter_ = f;
    filter_->setSampleRate(sampleRate_);
    if (upd)
    {
        calcResponse_();
        update();
    }
}

void FilterResponseWidget::setFftDisplay(bool fft, bool upd)
{
    doFft_ = fft;
    if (upd)
    {
        calcResponse_();
        update();
    }
}

void FilterResponseWidget::setNumBands(uint num, bool upd)
{
    numBands_ = num;
    if (upd)
    {
        calcResponse_();
        update();
    }
}

void FilterResponseWidget::setBufferSize(uint size, bool upd)
{
    bufferSize_ = size;
    if (upd)
    {
        calcResponse_();
        update();
    }
}

void FilterResponseWidget::mousePressEvent(QMouseEvent * e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        const F32 freq = freqForX(e->x());
        emit frequencyChanged(freq);
    }
}

void FilterResponseWidget::mouseMoveEvent(QMouseEvent * e)
{
    if (e->buttons() & Qt::LeftButton)
    {
        const F32 freq = freqForX(e->x());
        emit frequencyChanged(freq);
    }
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

    if (!thread_)
        thread_ = new ResponseCalc(this, response_);
    else
    {
        ((ResponseCalc*)thread_)->stop();
    }

    response_.resize(numBands_);

    if (doFft_)
        ((ResponseCalc*)thread_)->calcFft();
    else
        ((ResponseCalc*)thread_)->calc(0, response_.size()-1);
}

namespace
{
    void ResponseCalc::run()
    {
        filter = w->filter();

        if (doFft_)
            calcFft_();
        else
            calcResponse_();
    }


    void ResponseCalc::calc(uint from, uint to)
    {
        doStop = false;
        this->from = from;
        this->to = to;
        doFft_ = false;
        start();
    }

    void ResponseCalc::calcFft()
    {
        doStop = false;
        doFft_ = true;
        start();
    }

    void ResponseCalc::stop()
    {
        doStop = true;
        wait();
    }


    void ResponseCalc::calcResponse_()
    {
        std::vector<F32>
                buffer(w->bufferSize());

        for (uint j=from; j<=to; ++j)
        {
            if (doStop)
                return;

            const F32 freq = w->freqForBand(j);

            // create a sine tone
            const uint fadeoutp = buffer.size() * 0.8,
                       fadeoutl = buffer.size() - fadeoutp;
            for (uint i=0; i<buffer.size(); ++i)
            {
                // with fade-out
                F32 f = i < fadeoutp ? 1.f :
                             1.f - F32(i-fadeoutp) / fadeoutl;
                buffer[i] = f * std::sin(F32(i) / w->sampleRate() * TWO_PI * freq);
            }

            if (doStop)
                return;

            // filter it
            filter.reset();
            filter.process(&buffer[0], &buffer[0], buffer.size());

            // get amplitude
            F32 amp = std::abs(buffer[0]);
            for (uint i=1; i<buffer.size(); ++i)
                amp = std::max(amp, std::abs(buffer[i]));

            response[j] = amp;
        }

        w->update();
    }


    void ResponseCalc::calcFft_()
    {
        const uint bufferSize = 2048;

        std::vector<F32>
                buffer(bufferSize * 2);

        for (auto & v : response)
            v = 0.0;

        const uint num = 200;

        filter.reset();

        // repeat n times
        for (uint k = 0; k < num && !doStop; ++k)
        {

            // fill with noise
            for (uint j=0; j<bufferSize; ++j)
            {
                // window function
                const F32 fade = std::sin(F32(j) / bufferSize * PI);
                // noise
                buffer[j] = fade * (F32)rand() / RAND_MAX * 2.f - 1.f;
                //buffer[j] = fade * std::sin(F32(j)/w->sampleRate() * TWO_PI * 4000);
            }

            // filter it
            filter.process(&buffer[0], &buffer[0], bufferSize);

            // get fft
            MATH::real_fft(&buffer[0], bufferSize);
            MATH::get_amplitude_phase(&buffer[0], bufferSize);

            // fill into response array
            for (uint j=0; j<response.size(); ++j)
            {
                // index into ft
                uint i = (j * (bufferSize/2) / response.size());

                // get amplitude
                F32 amp = (buffer[i]
                // and take aliased frequencies above nyquist
                // (maybe this is more useful information)
                           + buffer[bufferSize-1-i]) * 0.75f;

                response[j] = std::max(response[j], amp);
            }
        }

        w->update();
    }
}


} // namespace GUI
} // namespace MO
