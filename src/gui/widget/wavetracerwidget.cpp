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
#include <QTime>
#include <QThread>

#include "wavetracerwidget.h"
#include "gui/widget/spinbox.h"
#include "gui/widget/doublespinbox.h"
#include "gui/widget/glslwidget.h"
#include "gui/widget/cameracontrolwidget.h"
#include "audio/spatial/wavetracershader.h"
#include "audio/tool/irmap.h"
#include "io/files.h"
#include "io/log.h"


namespace MO {
namespace GUI {

    /*
    class WaveTracerWidgetThread : public QThread
    {
    public:
        WaveTracerWidgetThread(WaveTracerWidget * w)
            : QThread       (w)
        {
            connect(&tracer, &AUDIO::WaveTracerShader::frameFinished, [=]()
            {
                img = tracer.getIrImage(res);
            });
        }

        void run() Q_DECL_OVERRIDE
        {
            doStop = false;

            tracer.start();

            while (!doStop)
            {


            }

            tracer.stop();
        }

        AUDIO::WaveTracerShader tracer;
        volatile bool doStop;
        QImage img, imgOut;
        QSize res;
    };
    */

    struct WaveTracerWidget::Private
    {
        Private(WaveTracerWidget * w)
            : widget            (w)
            , tracer            (new AUDIO::WaveTracerShader(widget))
        {
            updateTime.start();
        }

        void createWidgets();
        void updateFromWidgets();
        void updateFromLiveWidgets();
        void updateFromSettings();
        void updateVisibility();
        void updateImage();

        void requestIrImage();

        WaveTracerWidget * widget;

        AUDIO::WaveTracerShader * tracer;
        QTime updateTime;
        bool doNextUpdate;

        CameraControlWidget * camera;
        GlslWidget * source;
        SpinBox * sbMaxSteps,
                * sbMaxReflect,
                * sbNumSamples,
                * sbResX,
                * sbResY;
        DoubleSpinBox
                * sbSoundRad,
                * sbMaxDist,
                * sbReflect,
                * sbMicAngle,
                * sbBright,
                * sbEps,
                * sbDiffuse,
                * sbDiffuseRnd,
                * sbFresnel,
                * sbFudge,
                * sbColorX,
                * sbColorY,
                * sbColorZ;
        QComboBox
                * comboRender;
        QLabel * labelImage,
               * labelIr,
               * labelIrInfo;
        QPushButton
                * butSaveIr;
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
    connect(p_->tracer, SIGNAL(finishedIrImage(QImage)),
            this, SLOT(p_onIrImage_(QImage)), Qt::QueuedConnection);

    p_->tracer->start();
}

WaveTracerWidget::~WaveTracerWidget()
{
    p_->tracer->stop();

    delete p_;
}


void WaveTracerWidget::Private::createWidgets()
{
    auto lv0 = new QVBoxLayout(widget);

    auto lh0 = new QHBoxLayout();
    lv0->addLayout(lh0);

        // ----------- controlls -------------

        auto lv = new QVBoxLayout();
        lh0->addLayout(lv, 1);

            auto label = new QLabel(tr("ray marching"), widget);
            QFont font = label->font();
            font.setPointSizeF(font.pointSizeF() * 1.2);
            label->setFont(font);
            lv->addWidget(label);

            auto lh = new QHBoxLayout();
            lv->addLayout(lh);

                auto sb = sbResX = new SpinBox(widget);
                sb->setLabel(tr("resolution"));
                sb->setRange(16, 1024);
                connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
                lh->addWidget(sb);

                sb = sbResY = new SpinBox(widget);
                sb->setRange(16, 1024);
                connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
                lh->addWidget(sb);


            sb = sbMaxSteps = new SpinBox(widget);
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

            dsb = sbEps = new DoubleSpinBox(widget);
            dsb->setLabel(tr("normal estimation size"));
            dsb->setRange(0.00001, 10.);
            dsb->setDecimals(8);
            dsb->setSingleStep(0.0001);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbFudge = new DoubleSpinBox(widget);
            dsb->setLabel(tr("fudge factor"));
            dsb->setRange(0.0000001, 1.);
            dsb->setDecimals(8);
            dsb->setSingleStep(0.01);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            label = new QLabel(tr("visual options"), widget);
            label->setFont(font);
            lv->addWidget(label);

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

            lh = new QHBoxLayout();
            lv->addLayout(lh);

                dsb = sbColorX = new DoubleSpinBox(widget);
                dsb->setLabel(tr("sound/light color"));
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

            label = new QLabel(tr("reflection"), widget);
            label->setFont(font);
            lv->addWidget(label);

            dsb = sbReflect = new DoubleSpinBox(widget);
            dsb->setLabel(tr("global reflectiveness"));
            dsb->setRange(0., 10000.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.1);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbFresnel = new DoubleSpinBox(widget);
            dsb->setLabel(tr("fresnel-like reflectiveness"));
            dsb->setRange(0., 1.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.01);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbDiffuse = new DoubleSpinBox(widget);
            dsb->setLabel(tr("diffuse reflection"));
            dsb->setRange(0., 1.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.05);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbDiffuseRnd = new DoubleSpinBox(widget);
            dsb->setLabel(tr("randomize diffuse"));
            dsb->setRange(0., 1.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.05);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            label = new QLabel(tr("sound"), widget);
            label->setFont(font);
            lv->addWidget(label);

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

            camera = new CameraControlWidget(widget);
            connect(camera, SIGNAL(cameraMatrixChanged(MO::Mat4)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(camera);

            auto lv1 = new QVBoxLayout(camera);

                labelImage = new QLabel(widget);
                lv1->addWidget(labelImage);

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

            lv->addStretch(1);

            butSaveIr = new QPushButton("save impulse response", widget);
            connect(butSaveIr, SIGNAL(clicked()), widget, SLOT(saveIr()));
            lv->addWidget(butSaveIr);

    labelIr = new QLabel(widget);
    labelIr->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lv0->addWidget(labelIr);

    labelIrInfo = new QLabel(widget);
    lv0->addWidget(labelIrInfo);


}

void WaveTracerWidget::updateTracer()
{
    p_->doNextUpdate = true;

    if (!p_->tracer->isRunning())
        p_->tracer->start();
}

void WaveTracerWidget::Private::updateFromWidgets()
{
    auto s = tracer->settings();

    s.resolution = QSize(sbResX->value(), sbResY->value());
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
    s.diffuse = sbDiffuse->value();
    s.diffuseRnd = sbDiffuseRnd->value();
    s.fresnel = sbFresnel->value();
    s.fudge = sbFudge->value();
    s.soundColor.x = sbColorX->value();
    s.soundColor.y = sbColorY->value();
    s.soundColor.z = sbColorZ->value();
    s.camera = glm::inverse(camera->cameraMatrix());

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
    sbDiffuse->setValue(ls.diffuse);
    sbDiffuseRnd->setValue(ls.diffuseRnd);
    sbFresnel->setValue(ls.fresnel);
    sbFudge->setValue(ls.fudge);
    sbColorX->setValue(ls.soundColor.x);
    sbColorY->setValue(ls.soundColor.y);
    sbColorZ->setValue(ls.soundColor.z);
    camera->setCameraMatrix(ls.camera);

    const auto s = tracer->settings();

    sbResX->setValue(s.resolution.width());
    sbResY->setValue(s.resolution.height());
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
    //MO_DEBUG("WaveTracerWidget::updateImage()");

    QSize res = QSize(384, 384) * widget->devicePixelRatio();

    if (tracer->wasError())
        labelImage->setPixmap(QPixmap(res));
    else
    {
        labelImage->setPixmap(QPixmap::fromImage(tracer->getImage().scaled(res)));
    }
}

void WaveTracerWidget::Private::requestIrImage()
{
    tracer->requestIrImage(labelIr->size());
}

void WaveTracerWidget::saveIr()
{
    auto ir = p_->tracer->getIrMap();

    QString fn = IO::Files::getSaveFileName(IO::FT_SOUND_FILE, this);
    if (fn.isEmpty())
        return;

    ir.saveWav(fn, 48000);
}

void WaveTracerWidget::p_tracerFinished_()
{
    if (p_->tracer->wasError())
        QMessageBox::critical(this, tr("wave tracer error"), p_->tracer->errorString());

    if (p_->doNextUpdate || p_->updateTime.elapsed() > 500)
    {
        p_->requestIrImage();
        p_->updateImage();
        p_->updateTime.start();
        p_->doNextUpdate = false;
    }
}

void WaveTracerWidget::p_onLiveWidget_()
{
    p_->updateFromLiveWidgets();
}

void WaveTracerWidget::p_onWidget_()
{
    p_->updateFromWidgets();
}

void WaveTracerWidget::p_onIrImage_(QImage img)
{
    p_->labelIr->setPixmap(QPixmap::fromImage(img));
    p_->labelIrInfo->setText(p_->tracer->getIrInfo());
}

} // namespace GUI
} // namespace MO

