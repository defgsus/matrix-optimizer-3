/** @file wavetracerwidget.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 28.04.2015</p>
*/

#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QScrollArea>
#include <QMessageBox>
#include <QComboBox>

#include "wavetracerwidget.h"
#include "gui/widget/spinbox.h"
#include "gui/widget/doublespinbox.h"
#include "gui/widget/glslwidget.h"
#include "audio/spatial/wavetracershader.h"
#include "io/log.h"


namespace MO {
namespace GUI {


    struct WaveTracerWidget::Private
    {
        Private(WaveTracerWidget * w)
            : widget            (w)
            , tracer            (new AUDIO::WaveTracerShader(widget))
        {
        }

        void createWidgets();
        void updateFromWidgets();
        void updateFromLiveWidgets();
        void updateFromSettings();
        void updateVisibility();
        void updateImage();

        WaveTracerWidget * widget;

        AUDIO::WaveTracerShader * tracer;

        GlslWidget * source;
        SpinBox * sbMaxSteps,
                * sbMaxReflect,
                * sbNumSamples;
        DoubleSpinBox
                * sbSoundRad,
                * sbMaxDist,
                * sbReflect,
                * sbMicAngle,
                * sbBright,
                * sbEps,
                * sbColorX,
                * sbColorY,
                * sbColorZ;
        QComboBox
                * comboRender;
        QLabel * labelImage;
    };



WaveTracerWidget::WaveTracerWidget(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    p_->createWidgets();
    p_->updateFromSettings();

    connect(p_->tracer, SIGNAL(finished()),
            this, SLOT(p_tracerFinished_()), Qt::QueuedConnection);
    connect(p_->tracer, SIGNAL(frameFinished()),
            this, SLOT(p_tracerFinished_()), Qt::QueuedConnection);

    p_->tracer->start();
}

WaveTracerWidget::~WaveTracerWidget()
{
    p_->tracer->stop();

    delete p_;
}


void WaveTracerWidget::Private::createWidgets()
{
    auto lh0 = new QHBoxLayout(widget);

        // ----------- controlls -------------

        auto lv = new QVBoxLayout();
        lh0->addLayout(lv, 1);

            auto sb = sbMaxSteps = new SpinBox(widget);
            sb->setLabel(tr("maximum ray steps"));
            sb->setRange(1, 10000);
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
            lv->addWidget(sb);

            sb = sbMaxReflect = new SpinBox(widget);
            sb->setLabel(tr("maximum reflection steps"));
            sb->setRange(1, 10000);
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
            lv->addWidget(sb);

            auto dsb = sbMaxDist = new DoubleSpinBox(widget);
            dsb->setLabel(tr("maximum trace distance"));
            dsb->setRange(0., 10000.);
            dsb->setDecimals(3);
            dsb->setSingleStep(1.);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbReflect = new DoubleSpinBox(widget);
            dsb->setLabel(tr("global reflectiveness"));
            dsb->setRange(0., 10000.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.1);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbEps = new DoubleSpinBox(widget);
            dsb->setLabel(tr("normal estimation size"));
            dsb->setRange(0.00001, 10.);
            dsb->setDecimals(8);
            dsb->setSingleStep(0.0001);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbSoundRad = new DoubleSpinBox(widget);
            dsb->setLabel(tr("sound source radius"));
            dsb->setRange(0., 10000.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.1);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbMicAngle = new DoubleSpinBox(widget);
            dsb->setLabel(tr("microphone angle"));
            dsb->setRange(0., 360.);
            dsb->setDecimals(3);
            dsb->setSingleStep(1.);
            dsb->setSuffix("°");
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);


            sb = sbNumSamples = new SpinBox(widget);
            sb->setLabel(tr("number of multi-samples"));
            sb->setRange(1, 10000);
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
            lv->addWidget(sb);

            dsb = sbBright = new DoubleSpinBox(widget);
            dsb->setLabel(tr("brightness"));
            dsb->setRange(0., 10000.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.1);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            // color
            auto label = new QLabel(tr("sound/light color"), widget);
            lv->addWidget(label);

            auto lh = new QHBoxLayout();
            lv->addLayout(lh);

                dsb = sbColorX = new DoubleSpinBox(widget);
                dsb->setRange(0., 1.);
                dsb->setDecimals(3);
                dsb->setSingleStep(0.1);
                connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
                lh->addWidget(dsb);

                dsb = sbColorY = new DoubleSpinBox(widget);
                dsb->setRange(0., 1.);
                dsb->setDecimals(3);
                dsb->setSingleStep(0.1);
                connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
                lh->addWidget(dsb);

                dsb = sbColorZ = new DoubleSpinBox(widget);
                dsb->setRange(0., 1.);
                dsb->setDecimals(3);
                dsb->setSingleStep(0.1);
                connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
                lh->addWidget(dsb);

            lv->addStretch(1);

        // ------------ source ----------------

        lv = new QVBoxLayout();
        lh0->addLayout(lv, 2);

            source = new GlslWidget(widget);
            source->setUpdateOptional(true);
            connect(source, SIGNAL(scriptTextChanged()), widget, SLOT(p_onWidget_()));
            lv->addWidget(source);

        // ------------ image ----------------

        lv = new QVBoxLayout();
        lh0->addLayout(lv);

            label = new QLabel(tr("render mode"), widget);
            lv->addWidget(label);

            comboRender = new QComboBox(widget);
            comboRender->addItem(tr("path tracing audio"));
            comboRender->addItem(tr("path tracing debug"));
            comboRender->addItem(tr("ray tracing"));
            comboRender->addItem(tr("field slice"));
            connect(comboRender, SIGNAL(currentIndexChanged(int)),
                    widget, SLOT(p_onWidget_()));
            lv->addWidget(comboRender);

            labelImage = new QLabel(widget);
            lv->addWidget(labelImage);

            lv->addStretch(1);
}

void WaveTracerWidget::updateTracer()
{
    if (!p_->tracer->isRunning())
        p_->tracer->start();
}

void WaveTracerWidget::Private::updateFromWidgets()
{
    auto s = tracer->settings();

    //s.resolution =
    s.maxTraceStep = sbMaxSteps->value();
    s.maxReflectStep = sbMaxReflect->value();
    s.numMultiSamples = sbNumSamples->value();
    s.userCode = source->scriptText();
    s.renderMode = (AUDIO::WaveTracerShader::RenderMode)
                        std::max(0, comboRender->currentIndex());

    tracer->setSettings(s);

    widget->updateTracer();

    updateVisibility();
}

void WaveTracerWidget::Private::updateFromLiveWidgets()
{
    auto s = tracer->liveSettings();

    s.maxTraceDist = sbMaxDist->value();
    s.reflectivness = sbReflect->value();
    s.brightness = sbBright->value();
    s.micAngle = sbMicAngle->value();
    s.normalEps = sbEps->value();
    s.soundRadius = sbSoundRad->value();
    s.soundColor.x = sbColorX->value();
    s.soundColor.y = sbColorY->value();
    s.soundColor.z = sbColorZ->value();

    tracer->setLiveSettings(s);

    widget->updateTracer();
}

void WaveTracerWidget::Private::updateFromSettings()
{
    const auto ls = tracer->liveSettings();

    sbMaxDist->setValue(ls.maxTraceDist);
    sbReflect->setValue(ls.reflectivness);
    sbMicAngle->setValue(ls.micAngle);
    sbBright->setValue(ls.brightness);
    sbEps->setValue(ls.normalEps);
    sbSoundRad->setValue(ls.soundRadius);
    sbColorX->setValue(ls.soundColor.x);
    sbColorY->setValue(ls.soundColor.y);
    sbColorZ->setValue(ls.soundColor.z);

    const auto s = tracer->settings();

    comboRender->setCurrentIndex(s.renderMode);
    source->setScriptText(s.userCode);
    sbMaxSteps->setValue(s.maxTraceStep);
    sbMaxReflect->setValue(s.maxReflectStep);
    sbNumSamples->setValue(s.numMultiSamples);

    updateVisibility();
}

void WaveTracerWidget::Private::updateVisibility()
{
//    const auto ls = tracer->liveSettings();
    const auto s = tracer->settings();

    const bool
            wt = s.renderMode == AUDIO::WaveTracerShader::RM_WAVE_TRACER_VISIBLE,
            rt = s.renderMode == AUDIO::WaveTracerShader::RM_RAY_TRACER,
            fs = s.renderMode == AUDIO::WaveTracerShader::RM_FIELD_SLICE;

    sbNumSamples->setVisible(wt || fs);
    sbColorX->setVisible(rt || fs);
    sbColorY->setVisible(rt || fs);
    sbColorZ->setVisible(rt || fs);
}

void WaveTracerWidget::Private::updateImage()
{
    MO_DEBUG("WaveTracerWidget::updateImage()");

    if (tracer->wasError())
        labelImage->setPixmap(QPixmap(tracer->settings().resolution));
    else
        labelImage->setPixmap(QPixmap::fromImage(tracer->getImage()));
}

void WaveTracerWidget::p_tracerFinished_()
{
    if (p_->tracer->wasError())
        QMessageBox::critical(this, tr("wave tracer error"), p_->tracer->errorString());

    p_->updateImage();
}

void WaveTracerWidget::p_onLiveWidget_()
{
    p_->updateFromLiveWidgets();
}

void WaveTracerWidget::p_onWidget_()
{
    p_->updateFromWidgets();
}

} // namespace GUI
} // namespace MO
