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
            , tracerImg         (new AUDIO::WaveTracerShader(widget))
        {
            updateTime.start();
            updateTimeImg.start();

            auto s = tracerImg->settings();
            s.renderMode = AUDIO::WaveTracerShader::RM_WAVE_TRACER_VISIBLE;
            s.numPasses = 10;
            tracerImg->setSettings(s);

            s = tracer->settings();
            s.numPasses = 200;
            tracer->setSettings(s);
        }

        void createWidgets();
        void updateFromWidgets(bool doTracer, bool doTracerImg);
        void updateFromLiveWidgets(bool doTracer, bool doTracerImg);
        void updateFromSettings();
        void updateVisibility();
        void updateImage();

        void requestIrImage();

        WaveTracerWidget * widget;

        AUDIO::WaveTracerShader
            * tracer,
            * tracerImg;
        QTime updateTime, updateTimeImg;
        volatile bool doNextUpdate, doNextUpdateImg;

        CameraControlWidget * camera;
        GlslWidget * source;
        SpinBox * sbMaxSteps,
                * sbMaxReflect,
                * sbPasses,
                * sbImgPasses,
                * sbResX,
                * sbResY,
                * sbImgResX,
                * sbImgResY;;
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
                * sbRndRay,
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
    connect(p_->tracerImg, SIGNAL(finished()),
            this, SLOT(p_tracerImgFinished_()), Qt::QueuedConnection);
    connect(p_->tracerImg, SIGNAL(frameFinished()),
            this, SLOT(p_tracerImgFinished_()), Qt::QueuedConnection);

    updateTracer();
}

WaveTracerWidget::~WaveTracerWidget()
{
    p_->tracer->stop();
    p_->tracerImg->stop();

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

            // RAY_MARCHING

            auto label = new QLabel(tr("ray marching"), widget);
            QFont font = label->font();
            font.setPointSizeF(font.pointSizeF() * 1.2);
            label->setFont(font);
            lv->addWidget(label);

            auto lh = new QHBoxLayout();
            lv->addLayout(lh);

                auto sb = sbResX = new SpinBox(widget);
                sb->setLabel(tr("resolution"));
                sb->setRange(16, 1024*16);
                sb->setSingleStep(16);
                connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
                lh->addWidget(sb);

                sb = sbResY = new SpinBox(widget);
                sb->setRange(16, 1024*16);
                sb->setSingleStep(16);
                connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
                lh->addWidget(sb);

            sb = sbPasses = new SpinBox(widget);
            sb->setLabel(tr("number of passes"));
            sb->setRange(1, 10000);
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
            lv->addWidget(sb);

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

            dsb = sbRndRay = new DoubleSpinBox(widget);
            dsb->setLabel(tr("randomize ray dir"));
            dsb->setRange(0., 100.);
            dsb->setDecimals(5);
            dsb->setSingleStep(0.01);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            // REFLECTION

            label = new QLabel(tr("reflection"), widget);
            label->setFont(font);
            lv->addWidget(label);

            dsb = sbReflect = new DoubleSpinBox(widget);
            dsb->setLabel(tr("global reflectiveness"));
            dsb->setRange(0., 10000.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.05);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbFresnel = new DoubleSpinBox(widget);
            dsb->setLabel(tr("fresnel-like reflectiveness"));
            dsb->setRange(0., 1.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.05);
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

            // SOUND

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

            label = new QLabel(tr("visual options"), widget);
            label->setFont(font);
            lv->addWidget(label);

            comboRender = new QComboBox(widget);
            // XXX QVariants not used. order can't be changed right now.
            comboRender->addItem(tr("path tracing"), QVariant(AUDIO::WaveTracerShader::RM_WAVE_TRACER_VISIBLE));
            comboRender->addItem(tr("ray tracing"), QVariant(AUDIO::WaveTracerShader::RM_RAY_TRACER));
            comboRender->addItem(tr("field slice"), QVariant(AUDIO::WaveTracerShader::RM_FIELD_SLICE));
            connect(comboRender, SIGNAL(currentIndexChanged(int)),
                    widget, SLOT(p_onImgWidget_()));
            lv->addWidget(comboRender);

            lh = new QHBoxLayout();
                lv->addLayout(lh);

                sb = sbImgResX = new SpinBox(widget);
                sb->setLabel(tr("resolution"));
                sb->setRange(16, 1024);
                sb->setSingleStep(16);
                connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onImgWidget_()));
                lh->addWidget(sb);

                sb = sbImgResY = new SpinBox(widget);
                sb->setRange(16, 1024);
                sb->setSingleStep(16);
                connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onImgWidget_()));
                lh->addWidget(sb);

            sb = sbImgPasses = new SpinBox(widget);
            sb->setLabel(tr("number of passes"));
            sb->setRange(1, 10000);
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onImgWidget_()));
            lv->addWidget(sb);

            dsb = sbBright = new DoubleSpinBox(widget);
            dsb->setLabel(tr("sound brightness"));
            dsb->setRange(0., 10000.);
            dsb->setDecimals(3);
            dsb->setSingleStep(0.1);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onImgLiveWidget_()));
            lv->addWidget(dsb);

            // color

            lh = new QHBoxLayout();
            lv->addLayout(lh);

                dsb = sbColorX = new DoubleSpinBox(widget);
                dsb->setLabel(tr("sound/light color"));
                dsb->setRange(0., 1.);
                dsb->setDecimals(3);
                dsb->setSingleStep(0.1);
                connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onImgLiveWidget_()));
                lh->addWidget(dsb);

                dsb = sbColorY = new DoubleSpinBox(widget);
                dsb->setRange(0., 1.);
                dsb->setDecimals(3);
                dsb->setSingleStep(0.1);
                connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onImgLiveWidget_()));
                lh->addWidget(dsb);

                dsb = sbColorZ = new DoubleSpinBox(widget);
                dsb->setRange(0., 1.);
                dsb->setDecimals(3);
                dsb->setSingleStep(0.1);
                connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onImgLiveWidget_()));
                lh->addWidget(dsb);


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
    p_->doNextUpdateImg = true;

    if (!p_->tracer->isRunning())
        p_->tracer->start();

    if (!p_->tracerImg->isRunning())
        p_->tracerImg->start();
}

void WaveTracerWidget::Private::updateFromWidgets(bool doTracer, bool doTracerImg)
{
    if (doTracerImg)
    {
        auto s = tracerImg->settings();

        s.resolution = QSize(sbImgResX->value(), sbImgResY->value());
        s.maxTraceStep = sbMaxSteps->value();
        s.maxReflectStep = sbMaxReflect->value();
        s.doPassAverage = true;
        s.userCode = source->scriptText();
        s.renderMode = (AUDIO::WaveTracerShader::RenderMode)(
                            std::max(0, comboRender->currentIndex()) + 1);
        s.numPasses = sbImgPasses->value();

        tracerImg->setSettings(s);
    }

    if (doTracer)
    {
        auto s = tracer->settings();

        s.resolution = QSize(sbResX->value(), sbResY->value());
        s.maxTraceStep = sbMaxSteps->value();
        s.maxReflectStep = sbMaxReflect->value();
        s.doPassAverage = false;
        s.userCode = source->scriptText();
        s.renderMode = AUDIO::WaveTracerShader::RM_WAVE_TRACER;
        s.numPasses = sbPasses->value();

        tracer->setSettings(s);
    }

    widget->updateTracer();

    updateVisibility();
}

void WaveTracerWidget::Private::updateFromLiveWidgets(bool doTracer, bool doTracerImg)
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
    s.rndRay = sbRndRay->value();
    s.soundColor.x = sbColorX->value();
    s.soundColor.y = sbColorY->value();
    s.soundColor.z = sbColorZ->value();
    s.camera = glm::inverse(camera->cameraMatrix());

    if (doTracerImg)
        tracerImg->setLiveSettings(s);
    if (doTracer)
        tracer->setLiveSettings(s);

    widget->updateTracer();
}

void WaveTracerWidget::Private::updateFromSettings()
{
    const auto ls = tracerImg->liveSettings();

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
    sbRndRay->setValue(ls.rndRay);
    sbColorX->setValue(ls.soundColor.x);
    sbColorY->setValue(ls.soundColor.y);
    sbColorZ->setValue(ls.soundColor.z);
    camera->setCameraMatrix(glm::inverse(ls.camera));

    auto s = tracer->settings();

    sbResX->setValue(s.resolution.width());
    sbResY->setValue(s.resolution.height());
    source->setScriptText(s.userCode);
    sbMaxSteps->setValue(s.maxTraceStep);
    sbMaxReflect->setValue(s.maxReflectStep);
    sbPasses->setValue(s.numPasses);

    s = tracerImg->settings();

    sbImgResX->setValue(s.resolution.width());
    sbImgResY->setValue(s.resolution.height());
    comboRender->setCurrentIndex(s.renderMode-1);
    source->setScriptText(s.userCode);
    sbMaxSteps->setValue(s.maxTraceStep);
    sbMaxReflect->setValue(s.maxReflectStep);
    sbImgPasses->setValue(s.numPasses);

    updateVisibility();
}

void WaveTracerWidget::Private::updateVisibility()
{
//    const auto ls = tracer->liveSettings();
    const auto //s = tracer->settings(),
               si = tracerImg->settings();

    const bool
            //wt = si.renderMode == AUDIO::WaveTracerShader::RM_WAVE_TRACER_VISIBLE,
            rt = si.renderMode == AUDIO::WaveTracerShader::RM_RAY_TRACER,
            fs = si.renderMode == AUDIO::WaveTracerShader::RM_FIELD_SLICE;

    //sbImgPasses->setVisible(wt || fs);
    //sbBright->setVisible(true);
    sbColorX->setVisible(rt || fs);
    sbColorY->setVisible(rt || fs);
    sbColorZ->setVisible(rt || fs);
}

void WaveTracerWidget::Private::updateImage()
{
    //MO_DEBUG("WaveTracerWidget::updateImage()");

    QSize res = QSize(384, 384) * widget->devicePixelRatio();

    if (tracerImg->wasError())
        labelImage->setPixmap(QPixmap(res));
    else
    {
        labelImage->setPixmap(QPixmap::fromImage(tracerImg->getImage().scaled(res)));
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

    if (p_->doNextUpdate || p_->updateTime.elapsed() > 2000
        || p_->tracer->passCount() >= p_->tracer->settings().numPasses)
    {
        p_->requestIrImage();
        p_->updateTime.start();
        p_->doNextUpdate = false;
    }
}

void WaveTracerWidget::p_tracerImgFinished_()
{
    if (p_->tracerImg->wasError())
        QMessageBox::critical(this, tr("image wave tracer error"), p_->tracerImg->errorString());

    if (p_->doNextUpdateImg || p_->updateTimeImg.elapsed() > 300
        || p_->tracerImg->passCount() >= p_->tracerImg->settings().numPasses)
    {
        p_->updateImage();
        p_->updateTimeImg.start();
        p_->doNextUpdateImg = false;
    }
}

void WaveTracerWidget::p_onLiveWidget_()
{
    p_->updateFromLiveWidgets(true, true);
}

void WaveTracerWidget::p_onWidget_()
{
    p_->updateFromWidgets(true, true);
}

void WaveTracerWidget::p_onImgLiveWidget_()
{
    p_->updateFromLiveWidgets(false, true);
}

void WaveTracerWidget::p_onImgWidget_()
{
    p_->updateFromWidgets(false, true);
}

void WaveTracerWidget::p_onIrImage_(QImage img)
{
    p_->labelIr->setPixmap(QPixmap::fromImage(img));
    p_->labelIrInfo->setText(p_->tracer->infoString());
}

} // namespace GUI
} // namespace MO

