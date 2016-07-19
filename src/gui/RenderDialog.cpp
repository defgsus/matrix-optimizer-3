/** @file renderdialog.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/25/2015</p>
*/

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QFrame>
#include <QLineEdit>
#include <QProgressBar>
#include <QMessageBox>
#include <QFileInfo>
#include <QCheckBox>
#include <QImageWriter>
#include <QKeyEvent>
#include <QMainWindow>
#include <QScrollArea>

#include "RenderDialog.h"
#include "HelpDialog.h"
#include "widget/DoubleSpinBox.h"
#include "widget/SpinBox.h"
#include "widget/FilenameInput.h"
#include "engine/DiskRenderer.h"
#include "object/util/ObjectFactory.h"
#include "tool/stringmanip.h"
#include "io/DiskRenderSettings.h"
#include "io/error.h"
#include "io/Settings.h"
#include "io/log.h"

namespace MO {
namespace GUI {

/** @page render_settings
    <pre>

    - export directory

    - time
        - start
        - length / end

    - image
        - filename / pattern
        - number offset
        - file format
        - w, h
        - bits per pixel/channel and/or channel format
        - fps

    - audio
        - file format
        - filename / pattern
        - multichannel file ? or file per channel
        - samplerate
        - bits per channel (output!, intern it's 32 bit float)
        - ** microphone mapping **

    - ** video **

    </pre>

    @todo **
*/


struct RenderDialog::Private
{
    Private(RenderDialog * d)
        : diag          (d)
        , rendSet       (settings()->getDiskRenderSettings())
        , render        (0)
        , scene         (0)
        , timeUnit      (0)
        , ignoreWidgets (false)
        , outputWindow  (0)
    { }

    void createWidgets();
    QWidget * createHeadline(const QString& title);
    void updateFromWidgets();
    void updateFromSettings();
    void updateInfoLabels();
    void updateActivity(); // set the widget groups enables according to rendSet
    void createOutputWindow();

    RenderDialog * diag;

    DiskRenderSettings rendSet;
    DiskRenderer * render;
    QString sceneFilename;
    Scene * scene;
    int timeUnit;
    bool ignoreWidgets;

    QMainWindow * outputWindow;
    QLabel * outputLabel;
    QWidget
            * groupImage,
            * groupAudio,
            * groupAudioSplit;
    FilenameInput
            * editDir;
    QLineEdit
            * editImageName,
            * editAudioName;
    SpinBox
            * spinImageNum,
            * spinImageNumWidth,
            * spinImageW,
            * spinImageH,
            * spinImageFps,
            * spinImageQuality,
            * spinImageThreads,
            * spinImageQues,
            * spinAudioNum,
            * spinAudioNumWidth,
            * spinAudioChannels,
            * spinAudioBufferSize,
            * spinAudioSampleRate;
    DoubleSpinBox
            * spinStart,
            * spinLength;
    QComboBox
            * cbImageFormat,
            * cbAudioFormat;
    QCheckBox
            * checkImage,
            * checkImageComp,
            * checkAudio,
            * checkAudioSplit,
            * checkAudioNorm;
    QLabel
            * labelTime,
            * labelImageName,
            * labelAudioName,
            * labelQueSize,
            * labelProgress,
            * labelImage;
    QProgressBar
            * progBar;
    QPushButton
            * butGo,
            * butCancel;
};

RenderDialog::RenderDialog(const QString & sceneFilename, QWidget *parent)
    : QDialog       (parent)
    , p_            (new Private(this))
{
    setObjectName("_RenderDialog");
    setWindowTitle(tr("Render to disk (%1)").arg(QFileInfo(sceneFilename).fileName()));
    setMinimumSize(640, 640);

    settings()->restoreGeometry(this);

    p_->sceneFilename = sceneFilename;
    p_->createWidgets();
    p_->updateFromSettings();
}

RenderDialog::~RenderDialog()
{
    if (p_->outputWindow)
        settings()->storeGeometry(p_->outputWindow);
    settings()->storeGeometry(this);

    p_shutDown_();

    delete p_;
}

void RenderDialog::closeEvent(QEvent *)
{
    stopRender();
}

bool RenderDialog::isRendering() const
{
    return p_->render && p_->render->isRunning();
}

QWidget * RenderDialog::Private::createHeadline(const QString &title)
{
    auto l = new QLabel(title, diag);
    l->setMargin(10);
    QFont f(l->font());
    f.setPointSize(f.pointSize() * 1.34);
    f.setBold(true);
    l->setFont(f);
    return l;
}

void RenderDialog::Private::createWidgets()
{
    auto lv0 = new QVBoxLayout(diag);

        // output directory

        lv0->addWidget( createHeadline(tr("output directory")) );

        editDir = new FilenameInput(IO::FT_ANY, true, diag);
        editDir->setFilename(rendSet.directory());
        lv0->addWidget(editDir);
        connect(editDir, SIGNAL(filenameChanged(QString)),
                diag, SLOT(p_onWidget_()));

        auto frame = new QFrame(diag);
        frame->setFrameShape(QFrame::HLine);
        lv0->addWidget(frame);

        lv0->addWidget( createHeadline(tr("time range")) );

        // time info label
        labelTime = new QLabel(diag);
        lv0->addWidget(labelTime);

        // time range
        auto lh = new QHBoxLayout();
        lv0->addLayout(lh);

            auto cbTime = new QComboBox(diag);
            cbTime->addItem(tr("seconds"));
            cbTime->addItem(tr("frames"));
            cbTime->addItem(tr("samples"));
            connect(cbTime, SIGNAL(currentIndexChanged(int)),
                    diag, SLOT(p_onUnitChange_(int)));
            lh->addWidget(cbTime);

            spinStart = new DoubleSpinBox(diag);
            spinStart->setLabel(tr("start"));
            spinStart->setRange(-10e10, 10e10);
            spinStart->setDecimals(6);
            connect(spinStart, SIGNAL(valueChanged(double)),
                    diag, SLOT(p_onWidget_()));
            lh->addWidget(spinStart);

            spinLength = new DoubleSpinBox(diag);
            spinLength->setLabel(tr("length"));
            spinLength->setRange(0, 10e10);
            spinLength->setDecimals(6);
            connect(spinLength, SIGNAL(valueChanged(double)),
                    diag, SLOT(p_onWidget_()));
            lh->addWidget(spinLength);


        frame = new QFrame(diag);
        frame->setFrameShape(QFrame::HLine);
        lv0->addWidget(frame);

        lh = new QHBoxLayout();
        lv0->addLayout(lh);

            // ---- image ----

            auto lv = new QVBoxLayout();
            lh->addLayout(lv);

                auto lh1 = new QHBoxLayout();
                lh1->setMargin(0);
                lv->addLayout(lh1);

                    // enable?
                    checkImage = new QCheckBox(diag);
                    checkImage->setChecked(rendSet.imageEnable());
                    connect(checkImage, SIGNAL(stateChanged(int)),
                            diag, SLOT(p_onWidget_()));
                    lh1->addWidget(checkImage);

                    // heading
                    lh1->addWidget( createHeadline(tr("image settings")), 1);

                // sub-group for activity
                groupImage = new QWidget(diag);
                lv->addWidget(groupImage);
                lv = new QVBoxLayout(groupImage);
                lv->setMargin(0);

                    // name preview
                    labelImageName = new QLabel(diag);
                    lv->addWidget(labelImageName);

                    // name pattern
                    editImageName = new QLineEdit(diag);
                    editImageName->setText(rendSet.imagePattern());
                    lv->addWidget(editImageName);
                    connect(editImageName, SIGNAL(textChanged(QString)),
                            diag, SLOT(p_onWidget_()));

                    // frame number offset
                    spinImageNum = new SpinBox(diag);
                    spinImageNum->setLabel(tr("frame number offset"));
                    spinImageNum->setRange(0, 999999999);
                    spinImageNum->setValue(rendSet.imagePatternOffset());
                    lv->addWidget(spinImageNum);
                    connect(spinImageNum, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // frame number width
                    spinImageNumWidth = new SpinBox(diag);
                    spinImageNumWidth->setLabel(tr("frame number digits"));
                    spinImageNumWidth->setRange(0, 1000);
                    spinImageNumWidth->setValue(rendSet.imagePatternOffset());
                    lv->addWidget(spinImageNumWidth);
                    connect(spinImageNumWidth, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // compression
                    checkImageComp = new QCheckBox(tr("compression"), diag);
                    lv->addWidget(checkImageComp);
                    connect(checkImageComp, SIGNAL(stateChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // width
                    spinImageW = new SpinBox(diag);
                    spinImageW->setLabel(tr("width"));
                    spinImageW->setRange(0, 4096*4);
                    spinImageW->setValue(rendSet.imageWidth());
                    lv->addWidget(spinImageW);
                    connect(spinImageW, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // height
                    spinImageH = new SpinBox(diag);
                    spinImageH->setLabel(tr("height"));
                    spinImageH->setRange(0, 4096*4);
                    spinImageH->setValue(rendSet.imageHeight());
                    lv->addWidget(spinImageH);
                    connect(spinImageH, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // fps
                    spinImageFps = new SpinBox(diag);
                    spinImageFps->setLabel(tr("frames per second"));
                    spinImageFps->setRange(0, 60000);
                    spinImageFps->setValue(rendSet.imageFps());
                    lv->addWidget(spinImageFps);
                    connect(spinImageFps, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // format
                    cbImageFormat = new QComboBox(diag);
                    for (const DiskRenderSettings::ImageFormat & f
                         : DiskRenderSettings::imageFormats())
                        cbImageFormat->addItem(f.name, QVariant(int(f.index)));
                    cbImageFormat->setCurrentIndex(
                                rendSet.imageFormatIndex());
                    connect(cbImageFormat, SIGNAL(currentIndexChanged(int)),
                            diag, SLOT(p_onWidget_()));
                    lv->addWidget(cbImageFormat);

                    // quality
                    spinImageQuality = new SpinBox(diag);
                    spinImageQuality->setLabel(tr("image size [1-100]"));
                    spinImageQuality->setRange(1, 100);
                    spinImageQuality->setValue(rendSet.imageQuality());
                    lv->addWidget(spinImageQuality);
                    connect(spinImageQuality, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // threads
                    spinImageThreads = new SpinBox(diag);
                    spinImageThreads->setLabel(tr("storage threads"));
                    spinImageThreads->setRange(1, 256);
                    spinImageThreads->setValue(rendSet.imageNumThreads());
                    lv->addWidget(spinImageThreads);
                    connect(spinImageThreads, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // que
                    spinImageQues = new SpinBox(diag);
                    spinImageQues->setLabel(tr("max images in que"));
                    spinImageQues->setRange(1, 256);
                    spinImageQues->setValue(rendSet.imageNumQue());
                    lv->addWidget(spinImageQues);
                    connect(spinImageQues, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // que size label
                    labelQueSize = new QLabel(diag);
                    labelQueSize->setAlignment(Qt::AlignRight);
                    lv->addWidget(labelQueSize);

                    lv->addStretch(1);

            lh->addSpacing(8);

            // ---- audio ----

            lv = new QVBoxLayout();
            auto lv23 = lv; // need it later
            lh->addLayout(lv);

                lh1 = new QHBoxLayout();
                lh1->setMargin(0);
                lv->addLayout(lh1);

                    // enable?
                    checkAudio = new QCheckBox(diag);
                    checkAudio->setChecked(rendSet.audioEnable());
                    connect(checkAudio, SIGNAL(stateChanged(int)),
                            diag, SLOT(p_onWidget_()));
                    lh1->addWidget(checkAudio);

                    // heading
                    lh1->addWidget( createHeadline(tr("audio settings")) );

                // sub-group for activity
                groupAudio = new QWidget(diag);
                lv->addWidget(groupAudio);
                lv = new QVBoxLayout(groupAudio);
                lv->setMargin(0);

                    // name preview
                    labelAudioName = new QLabel(diag);
                    lv->addWidget(labelAudioName);

                    // name pattern
                    editAudioName = new QLineEdit(diag);
                    editAudioName->setText(rendSet.audioPattern());
                    lv->addWidget(editAudioName);
                    connect(editAudioName, SIGNAL(textChanged(QString)),
                            diag, SLOT(p_onWidget_()));

                    // frame number offset
                    spinAudioNum = new SpinBox(diag);
                    spinAudioNum->setLabel(tr("frame number offset"));
                    spinAudioNum->setRange(0, 999999999);
                    spinAudioNum->setValue(rendSet.audioPatternOffset());
                    lv->addWidget(spinAudioNum);
                    connect(spinAudioNum, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    // frame number width
                    spinAudioNumWidth = new SpinBox(diag);
                    spinAudioNumWidth->setLabel(tr("frame number digits"));
                    spinAudioNumWidth->setRange(0, 1000);
                    spinAudioNumWidth->setValue(rendSet.audioPatternOffset());
                    lv->addWidget(spinAudioNumWidth);
                    connect(spinAudioNumWidth, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    spinAudioChannels = new SpinBox(diag);
                    spinAudioChannels->setLabel(tr("number channels"));
                    spinAudioChannels->setRange(1, 1<<30);
                    lv->addWidget(spinAudioChannels);
                    connect(spinAudioChannels, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    spinAudioSampleRate = new SpinBox(diag);
                    spinAudioSampleRate->setLabel(tr("sampling rate (Hz)"));
                    spinAudioSampleRate->setRange(1, 1<<30);
                    lv->addWidget(spinAudioSampleRate);
                    connect(spinAudioSampleRate, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                    spinAudioBufferSize = new SpinBox(diag);
                    spinAudioBufferSize->setLabel(tr("buffer size"));
                    spinAudioBufferSize->setRange(1, 1<<30);
                    lv->addWidget(spinAudioBufferSize);
                    connect(spinAudioBufferSize, SIGNAL(valueChanged(int)),
                            diag, SLOT(p_onWidget_()));

                // -- split --

                checkAudioSplit = new QCheckBox(tr("split into single files"), diag);
                checkAudioSplit->setChecked(rendSet.audioSplitEnable());
                connect(checkAudioSplit, SIGNAL(stateChanged(int)),
                        diag, SLOT(p_onWidget_()));
                lv23->addWidget(checkAudioSplit);

                // sub-group for activity
                groupAudioSplit = new QWidget(diag);
                lv23->addWidget(groupAudioSplit);
                lv = new QVBoxLayout(groupAudioSplit);
                lv->setMargin(0);

                    checkAudioNorm = new QCheckBox(tr("normalize"), diag);
                    checkAudioNorm->setChecked(rendSet.audioNormalizeEnable());
                    connect(checkAudioNorm, SIGNAL(stateChanged(int)),
                            diag, SLOT(p_onWidget_()));
                    lv->addWidget(checkAudioNorm);

                    // format
                    cbAudioFormat = new QComboBox(diag);
                    for (const DiskRenderSettings::AudioFormat & f
                         : DiskRenderSettings::audioFormats())
                        cbAudioFormat->addItem(f.name, QVariant(int(f.index)));
                    cbAudioFormat->setCurrentIndex(
                                rendSet.audioFormatIndex());
                    connect(cbAudioFormat, SIGNAL(currentIndexChanged(int)),
                            diag, SLOT(p_onWidget_()));
                    lv->addWidget(cbAudioFormat);

                    lv->addStretch(1);

        // [Go!] [Cancel] buttons

        frame = new QFrame(diag);
        frame->setFrameShape(QFrame::HLine);
        lv0->addWidget(frame);

        lh = new QHBoxLayout();
        lv0->addLayout(lh);

            butGo = new QPushButton(tr("Go!"), diag);
            butGo->setStatusTip(tr("Starts rendering the scene with selected settings"));
            butGo->setDefault(true);
            connect(butGo, SIGNAL(pressed()), diag, SLOT(startRender()));
            lh->addWidget(butGo);

            butCancel = new QPushButton(tr("Close"), diag);
            butCancel->setStatusTip(tr("Stops rendering or closes the dialog"));
            connect(butCancel, &QPushButton::pressed, [=]()
            {
                if (diag->isRendering())
                    diag->stopRender();
                else
                    diag->reject();
            });
            lh->addWidget(butCancel);

        // progress bar
        progBar = new QProgressBar(diag);
        progBar->setVisible(false);
        lv0->addWidget(progBar);

        lh = new QHBoxLayout();
        lv0->addLayout(lh);

            labelProgress = new QLabel(diag);
            lh->addWidget(labelProgress);

            labelImage = new QLabel(diag);
            lh->addWidget(labelImage);
}

void RenderDialog::Private::updateFromWidgets()
{
    rendSet.setDirectory(editDir->filename());

    // ----------- image -----------

    rendSet.setImageEnable(checkImage->isChecked());
    rendSet.setImagePattern(editImageName->text());
    rendSet.setImagePatternOffset(spinImageNum->value());
    rendSet.setImagePatternWidth(spinImageNumWidth->value());
    rendSet.setImageSize(spinImageW->value(), spinImageH->value());
    rendSet.setImageFps(spinImageFps->value());
    rendSet.setImageFormat(cbImageFormat->currentIndex());
    rendSet.setImageQuality(spinImageQuality->value());
    rendSet.setImageCompression(checkImageComp->isChecked());
    rendSet.setImageNumThreads(spinImageThreads->value());
    rendSet.setImageNumQue(spinImageQues->value());

    // ----------- audio -----------

    rendSet.setAudioEnable(checkAudio->isChecked());
    rendSet.setAudioSplitEnable(checkAudioSplit->isChecked());
    rendSet.setAudioNormalizeEnable(checkAudioNorm->isChecked());
    rendSet.setAudioPattern(editAudioName->text());
    rendSet.setAudioPatternOffset(spinAudioNum->value());
    rendSet.setAudioPatternWidth(spinAudioNumWidth->value());
    rendSet.setAudioFormat(cbAudioFormat->currentIndex());
    rendSet.audioConfig().setBufferSize(spinAudioBufferSize->value());
    rendSet.audioConfig().setNumChannelsOut(spinAudioChannels->value());
    rendSet.audioConfig().setSampleRate(spinAudioSampleRate->value());

    switch (timeUnit)
    {
        case 0: rendSet.setStartSecond(spinStart->value());
                rendSet.setLengthSecond(spinLength->value());
        break;
        case 1: rendSet.setStartFrame(spinStart->value());
                rendSet.setLengthFrame(spinLength->value());
        break;
        case 2: rendSet.setStartSample(spinStart->value());
                rendSet.setLengthSample(spinLength->value());
        break;
    }

    updateInfoLabels();
    updateActivity();
}

void RenderDialog::Private::updateFromSettings()
{
    ignoreWidgets = true;

    editDir->setFilename(rendSet.directory());

    // -- image --

    checkImage->setChecked(rendSet.imageEnable());
    editImageName->setText(rendSet.imagePattern());
    spinImageNum->setValue(rendSet.imagePatternOffset());
    spinImageNumWidth->setValue(rendSet.imagePatternWidth());
    spinImageW->setValue(rendSet.imageWidth());
    spinImageH->setValue(rendSet.imageHeight());
    spinImageFps->setValue(rendSet.imageFps());
    cbImageFormat->setCurrentIndex(rendSet.imageFormatIndex());
    spinImageQuality->setValue(rendSet.imageQuality());
    checkImageComp->setChecked(rendSet.imageCompression());
    spinImageThreads->setValue(rendSet.imageNumThreads());
    spinImageQues->setValue(rendSet.imageNumQue());

    // -- audio --

    checkAudio->setChecked(rendSet.audioEnable());
    checkAudioSplit->setChecked(rendSet.audioSplitEnable());
    checkAudioNorm->setChecked(rendSet.audioNormalizeEnable());
    editAudioName->setText(rendSet.audioPattern());
    spinAudioNum->setValue(rendSet.audioPatternOffset());
    spinAudioNumWidth->setValue(rendSet.audioPatternWidth());
    cbAudioFormat->setCurrentIndex(rendSet.audioFormatIndex());
    spinAudioBufferSize->setValue(rendSet.audioConfig().bufferSize());
    spinAudioChannels->setValue(rendSet.audioConfig().numChannelsOut());
    spinAudioSampleRate->setValue(rendSet.audioConfig().sampleRate());

    switch (timeUnit)
    {
        case 0: spinStart->setValue(rendSet.startSecond());
                spinLength->setValue(rendSet.lengthSecond());
        break;
        case 1: spinStart->setValue(rendSet.startFrame());
                spinLength->setValue(rendSet.lengthFrame());
        break;
        case 2: spinStart->setValue(rendSet.startSample());
                spinLength->setValue(rendSet.lengthSample());
        break;
    }

    ignoreWidgets = false;

    updateInfoLabels();
    updateActivity();
}

void RenderDialog::Private::updateInfoLabels()
{
    /*
    switch (timeUnit)
    {
        case 0: labelTime->setText(tr("time %1 - %2")
                        .arg(time_to_string(rendSet.startSecond()))
                        .arg(time_to_string(rendSet.lengthSecond() - rendSet.startSecond())));
        break;
        case 1: labelTime->setText(tr("time %1 - %2")
                        .arg(rendSet.startFrame())
                        .arg(rendSet.lengthFrame() - rendSet.startFrame()));
        break;
        case 2: labelTime->setText(tr("time %1 - %2")
                        .arg(rendSet.startSample())
                        .arg(rendSet.lengthSample() - rendSet.startSample()));
        break;
    }
    */

    labelTime->setText(tr("time %1 - %2 / frames %3 - %4 / samples %5 - %6")
                        .arg(time_to_string(rendSet.startSecond()))
                        .arg(time_to_string(rendSet.endSecond()))
                        .arg(rendSet.startFrame())
                        .arg(rendSet.endFrame())
                        .arg(rendSet.startSample())
                        .arg(rendSet.endSample()));

    labelImageName->setText(tr("example name:") + "\n" + rendSet.makeImageFilename(rendSet.startFrame()));
    labelAudioName->setText(tr("example name:") + "\n" + rendSet.makeAudioFilename(0));

    labelQueSize->setText(tr("max image memory: %1")
                          .arg(byte_to_string(rendSet.imageSizeBytes()
                                              * (rendSet.imageNumQue() + rendSet.imageNumThreads()))));
}

void RenderDialog::Private::updateActivity()
{
    groupImage->setEnabled(rendSet.imageEnable());
    groupAudio->setEnabled(rendSet.audioEnable());
    groupAudioSplit->setEnabled(rendSet.audioSplitEnable());
    butGo->setEnabled( rendSet.imageEnable()
                       || rendSet.audioEnable()
                       || rendSet.audioSplitEnable() );

    QImageWriter w("./bla", rendSet.imageFormatExt().toUtf8());
    spinImageQuality->setVisible(w.supportsOption(QImageIOHandler::Quality));
    checkImageComp->setVisible(w.supportsOption(QImageIOHandler::CompressionRatio));
}

void RenderDialog::p_onWidget_()
{
    if (p_->ignoreWidgets)
        return;

    p_->updateFromWidgets();
}

void RenderDialog::p_onUnitChange_(int idx)
{
    p_->timeUnit = std::max(0, std::min(2, idx ));

    p_->updateFromSettings();
}

void RenderDialog::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Qt::Key_F1)
    {
        auto help = new HelpDialog("diskrender", this);
        help->show();
    }
    QDialog::keyPressEvent(e);
}

void RenderDialog::error(const QString & e)
{
    MO_DEBUG("Render Error:\n" << e);
    QMessageBox::critical(0, tr("Disk renderer"), e);
}


void RenderDialog::stopRender()
{
    if (!p_->render)
        return;

    p_->render->stop();
    p_->progBar->setVisible(false);

    p_shutDown_();
}

void RenderDialog::startRender()
{
    if (p_->render)
        return;

    p_->labelProgress->setText(tr("start rendering..."));
    p_->butCancel->setText(tr("Stop"));

    p_->render = new DiskRenderer(this);
    // on progress
    connect(p_->render, SIGNAL(progress(int)),
            this, SLOT(p_onProgress_(int)), Qt::QueuedConnection);
    connect(p_->render, SIGNAL(newImage(QImage)),
            this, SLOT(p_onImage_(QImage)), Qt::QueuedConnection);
    // on finished/failure
    connect(p_->render, SIGNAL(finished()),
            this, SLOT(p_onFinished_()), Qt::QueuedConnection);

    p_->render->setSettings(p_->rendSet);
    p_->render->setSceneFilename(p_->sceneFilename);
    /*if (!p_->render->loadScene(p_->sceneFilename))
    {
        error(tr("Error loading scene.\n").arg(p_->render->errorString()));
        p_->render->deleteLater();
    }*/

    p_->progBar->setValue(0);
    p_->progBar->setVisible(true);
    p_->labelProgress->clear();
    p_->render->start();
}

void RenderDialog::p_onFinished_()
{
    p_->progBar->setVisible(false);
    p_->butCancel->setText(tr("Close"));

    if (p_->render)
        if (!p_->render->ok())
            error(p_->render->errorString());

    p_shutDown_();
}

void RenderDialog::p_onProgress_(int p)
{
    p_->progBar->setValue(p);
    if (p_->render)
        p_->labelProgress->setText(p_->render->progressString());
}

void RenderDialog::p_shutDown_()
{
    if (p_->render)
        p_->render->deleteLater();
    p_->render = 0;
}

void RenderDialog::p_onImage_(const QImage & img)
{
    if (!p_->outputWindow)
        p_->createOutputWindow();
    if (p_->outputWindow->isVisible())
        p_->outputLabel->setPixmap(QPixmap::fromImage(img));
}

void RenderDialog::Private::createOutputWindow()
{
    outputWindow = new QMainWindow(diag);
    outputWindow->setObjectName(tr("_RendererOutput"));
    outputWindow->setWindowTitle(tr("Renderer preview"));
    outputWindow->setMinimumSize(200,200);
    settings()->restoreGeometry(outputWindow);

    //auto s = new QScrollArea(outputWindow);
    //outputWindow->setCentralWidget(s);

    outputLabel = new QLabel(outputWindow);
    //s->setWidget(outputLabel);
    outputWindow->setCentralWidget(outputLabel);

    outputWindow->show();
}

} // namespace GUI
} // namespace MO
