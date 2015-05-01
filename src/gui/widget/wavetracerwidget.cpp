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
#include "audio/tool/soundfile.h"
#include "audio/tool/soundfilemanager.h"
#include "audio/audioplayer.h"
#include "audio/audioplayerdata.h"
#include "io/files.h"
#include "io/settings.h"
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
            , soundFile         (0)
        {
            updateTime.start();
            updateTimeImg.start();

            auto s = tracerImg->settings();
            s.userCode = settings()->value("WaveTracer/source", s.userCode).toString()                                                                                                                                                                                                                                                                                                                                                                                                                                                                      ;
            s.renderMode = AUDIO::WaveTracerShader::RM_WAVE_TRACER_VISIBLE;
            s.numPasses = 10;
            tracerImg->setSettings(s);

            auto s1 = tracer->settings();
            s1.userCode = s.userCode;
            s1.numPasses = 200;
            tracer->setSettings(s1);
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

        AUDIO::SoundFile * soundFile;

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
                * sbRayDist,
                * sbTraceDist,
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
        QCheckBox
                * cbPhaseFlip;
        QLabel * labelImage,
               * labelIr,
               * labelIrInfo;
        QPushButton
                * butSaveIr,
                * butPlayIr;
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

    AUDIO::AudioPlayer::close();

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
            sb->setRange(1, 1<<30);
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onPasses_()));
            lv->addWidget(sb);

            sb = sbMaxSteps = new SpinBox(widget);
            sb->setLabel(tr("maximum ray steps"));
            sb->setRange(1, 10000);
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
            lv->addWidget(sb);

            auto dsb = sbRayDist = new DoubleSpinBox(widget);
            dsb->setLabel(tr("maximum ray length"));
            dsb->setRange(0., 10000.);
            dsb->setDecimals(3);
            dsb->setSingleStep(1.);
            connect(dsb, SIGNAL(valueChanged(double)), widget, SLOT(p_onLiveWidget_()));
            lv->addWidget(dsb);

            dsb = sbTraceDist = new DoubleSpinBox(widget);
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

            sb = sbMaxReflect = new SpinBox(widget);
            sb->setLabel(tr("maximum reflection steps"));
            sb->setRange(1, 10000);
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(p_onWidget_()));
            lv->addWidget(sb);

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

            auto cb = cbPhaseFlip = new QCheckBox(tr("flip phase"), widget);
            connect(cb, SIGNAL(clicked()), widget, SLOT(p_phase_()));
            lv->addWidget(cb);

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

            lh = new QHBoxLayout();
            lv->addLayout(lh);

                auto but = new QPushButton("play IR", widget);
                connect(but, SIGNAL(clicked()), widget, SLOT(playIr()));
                lh->addWidget(but);

                but = new QPushButton("load", widget);
                connect(but, SIGNAL(clicked()), widget, SLOT(loadSound()));
                lh->addWidget(but);

                but = new QPushButton("play", widget);
                connect(but, SIGNAL(clicked()), widget, SLOT(playSound()));
                lh->addWidget(but);

                but = new QPushButton("stop", widget);
                connect(but, &QPushButton::clicked, [](){ AUDIO::AudioPlayer::close(); });
                lh->addWidget(but);

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

        settings()->setValue("WaveTracer/source", s.userCode);

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

    s.maxTraceDist = sbTraceDist->value();
    s.maxRayDist = sbRayDist->value();
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

    sbTraceDist->setValue(ls.maxTraceDist);
    sbRayDist->setValue(ls.maxRayDist);
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

void WaveTracerWidget::playIr()
{
    auto ir = p_->tracer->getIrMap();

    if (!AUDIO::AudioPlayer::open())
        return;

    auto sam = ir.getSamples(AUDIO::AudioPlayer::sampleRate());
    auto data = AUDIO::AudioPlayerSample::fromData(&sam[0], sam.size(),
                    AUDIO::AudioPlayer::sampleRate());
    AUDIO::AudioPlayer::play(data);
    data->releaseRef();
}

bool WaveTracerWidget::loadSound()
{
    QString fn = IO::Files::getOpenFileName(IO::FT_SOUND_FILE, this);
    if (fn.isEmpty())
        return false;

    if (p_->soundFile)
        AUDIO::SoundFileManager::releaseSoundFile(p_->soundFile);

    p_->soundFile = AUDIO::SoundFileManager::getSoundFile(fn);
    return p_->soundFile != 0;
}

void WaveTracerWidget::playSound()
{
    if (!AUDIO::AudioPlayer::open())
        return;

    if (!p_->soundFile)
        if (!loadSound())
            return;
/*
    std::vector<F32> sam(88200, 0.f);
    for (size_t i=0; i<sam.size(); ++i)
    {
        F32 t = F32(i) / 44100.f;
        sam[i] = sin(t * 6.28f * 440.f) * (1.f - t);
    }
    auto data = AUDIO::AudioPlayerSample::fromData(
                    &sam[0], sam.size(),
                    AUDIO::AudioPlayer::sampleRate());
    AUDIO::AudioPlayer::play(data);
    data->releaseRef();
*/

    auto sam = p_->soundFile->getSamples();
    auto irmap = p_->tracer->getIrMap();
    auto ir = irmap.getSamples(AUDIO::AudioPlayer::sampleRate());

    std::vector<F32> conv(sam.size() + ir.size(), 0.f);

    F32 ma = 0.000001;
    for (size_t i=0; i<ir.size(); ++i)
        if (std::abs(ir[i]) > 0.00001)
        for (size_t j=0; j<sam.size(); ++j)
        {
            conv[j + i] += ir[i] * sam[j];
            ma = std::max(ma, conv[j+i]);
        }
    for (auto & s : conv)
        s /= ma;

    auto data = AUDIO::AudioPlayerSample::fromData(&conv[0], conv.size(),
                    AUDIO::AudioPlayer::sampleRate());
    AUDIO::AudioPlayer::play(data);
    data->releaseRef();
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

void WaveTracerWidget::p_onPasses_()
{
    uint num = p_->sbPasses->value();
    if (!p_->tracer->isRunning())
      //|| p_->tracer->passCount() < num)
        p_->updateFromWidgets(true, true);
    else
        p_->tracer->setNumPasses(num);
}

void WaveTracerWidget::p_phase_()
{
    bool f = p_->cbPhaseFlip->isChecked();
    p_->tracer->setFlipPhase(f);
    if (!p_->tracer->isRunning())
        p_->requestIrImage();
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

