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

#include "renderdialog.h"
#include "widget/doublespinbox.h"
#include "widget/spinbox.h"
#include "widget/filenameinput.h"
#include "engine/diskrenderer.h"
#include "object/objectfactory.h"
#include "tool/stringmanip.h"
#include "io/diskrendersettings.h"
#include "io/error.h"
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
        , render        (0)
        , scene         (0)
        , timeUnit      (0)
        , ignoreWidgets (false)
    { }

    void createWidgets();
    void updateFromWidgets();
    void updateFromSettings();
    void updateInfoLabels();

    RenderDialog * diag;

    DiskRenderSettings rendSet;
    DiskRenderer * render;
    QString sceneFilename;
    Scene * scene;
    int timeUnit;
    bool ignoreWidgets;

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
            * spinImageFps;
    DoubleSpinBox
            * spinStart,
            * spinLength;
    QComboBox
            * cbImageFormat;
    QLabel
            * labelTime,
            * labelImageName,
            * labelProgress;
    QProgressBar
            * progBar;
};

RenderDialog::RenderDialog(const QString & sceneFilename, QWidget *parent)
    : QDialog       (parent)
    , p_            (new Private(this))
{
    setObjectName("_RenderDialog");
    setMinimumSize(640, 640);

    p_->sceneFilename = sceneFilename;
    p_->createWidgets();
    p_->updateFromSettings();
}

RenderDialog::~RenderDialog()
{
    p_shutDown_();

    delete p_;
}

void RenderDialog::closeEvent(QEvent *)
{
    stopRender();
}

void RenderDialog::Private::createWidgets()
{
    auto lv0 = new QVBoxLayout(diag);

        // output directory

        editDir = new FilenameInput(IO::FT_ANY, true, diag);
        editDir->setFilename(rendSet.directory());
        lv0->addWidget(editDir);
        connect(editDir, SIGNAL(filenameChanged(QString)),
                diag, SLOT(p_onWidget_()));

        // name preview
        labelImageName = new QLabel(diag);
        lv0->addWidget(labelImageName);

        // time info label
        labelTime = new QLabel(diag);
        lv0->addWidget(labelTime);

        auto frame = new QFrame(diag);
        frame->setFrameShape(QFrame::HLine);
        lv0->addWidget(frame);

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

        lv0->addStretch(1);

        // [Go!] [Cancel] buttons

        frame = new QFrame(diag);
        frame->setFrameShape(QFrame::HLine);
        lv0->addWidget(frame);

        lh = new QHBoxLayout();
        lv0->addLayout(lh);

        auto but = new QPushButton(tr("Go!"), diag);
        but->setStatusTip(tr("Starts rendering the scene with selected settings"));
        but->setDefault(true);
        connect(but, SIGNAL(pressed()), diag, SLOT(render()));
        lh->addWidget(but);

        but = new QPushButton(tr("Cancel"), diag);
        but->setStatusTip(tr("Closes the dialog"));
        connect(but, SIGNAL(pressed()), diag, SLOT(stopRender()));
        lh->addWidget(but);

        // progress bar
        progBar = new QProgressBar(diag);
        progBar->setVisible(false);
        lv0->addWidget(progBar);

        labelProgress = new QLabel(diag);
        lv0->addWidget(labelProgress);
}

void RenderDialog::Private::updateFromWidgets()
{
    rendSet.setDirectory(editDir->filename());

    rendSet.setImagePattern(editImageName->text());
    rendSet.setImagePatternOffset(spinImageNum->value());
    rendSet.setImagePatternWidth(spinImageNumWidth->value());
    rendSet.setImageSize(spinImageW->value(), spinImageH->value());
    rendSet.setImageFps(spinImageFps->value());
    rendSet.setImageFormat(cbImageFormat->currentIndex());

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
}

void RenderDialog::Private::updateFromSettings()
{
    ignoreWidgets = true;

    editDir->setFilename(rendSet.directory());

    // -- image --

    editImageName->setText(rendSet.imagePattern());
    spinImageNum->setValue(rendSet.imagePatternOffset());
    spinImageNumWidth->setValue(rendSet.imagePatternWidth());
    spinImageW->setValue(rendSet.imageWidth());
    spinImageH->setValue(rendSet.imageHeight());
    spinImageFps->setValue(rendSet.imageFps());
    cbImageFormat->setCurrentIndex(rendSet.imageFormatIndex());

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
}

void RenderDialog::Private::updateInfoLabels()
{
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

    labelImageName->setText(tr("example name: ") + rendSet.makeImageFilename(0));
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


void RenderDialog::error(const QString & e)
{
    MO_PRINT("Render Error:\n" << e);
    /// @todo this might come from the render thread
    /// in which case the QMessageBox will not work or crash
    //QMessageBox::critical(0, tr("Disk renderer"), e);
}

void RenderDialog::stopRender()
{
    if (!p_->render)
        return;

    p_->render->stop();
    p_->progBar->setVisible(false);

    p_shutDown_();
}

void RenderDialog::render()
{
    if (p_->render)
        return;
    /*
    try
    {
        p_->scene = ObjectFactory::loadScene(p_->sceneFilename);
    }
    catch (const Exception& e)
    {
        error(tr("Error loading scene.\n").arg(e.what()));
        return;
    }*/

    p_->render = new DiskRenderer(this);
    // on progress
    connect(p_->render, &DiskRenderer::progress, [=](int p)
    {
        p_->progBar->setValue(p);
        p_->labelProgress->setText(p_->render->progressString());
    });
    // on finished/failure
    connect(p_->render, &DiskRenderer::finished, [this]()
    {
        p_->progBar->setVisible(false);
        if (!p_->render->ok())
            error(p_->render->errorString());
        p_shutDown_();
    });

    p_->render->setSettings(p_->rendSet);
    p_->render->setSceneFilename(p_->sceneFilename);
    /*if (!p_->render->loadScene(p_->sceneFilename))
    {
        error(tr("Error loading scene.\n").arg(p_->render->errorString()));
        p_->render->deleteLater();
    }*/

    p_->progBar->setVisible(true);
    p_->render->start();

    //accept();
}

void RenderDialog::p_shutDown_()
{
    if (p_->render)
        p_->render->deleteLater();
    p_->render = 0;
}


} // namespace GUI
} // namespace MO
