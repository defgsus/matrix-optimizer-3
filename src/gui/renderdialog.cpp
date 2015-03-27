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

#include "renderdialog.h"
#include "widget/doublespinbox.h"
#include "widget/spinbox.h"
#include "widget/filenameinput.h"
#include "io/diskrendersettings.h"

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
    Private(RenderDialog * d) : diag(d) { }

    void createWidgets();
    void updateFromWidgets();

    RenderDialog * diag;

    DiskRenderSettings rendSet;

    FilenameInput
            * editDir_;
    QLineEdit
            * editImageName_,
            * editAudioName_;
    SpinBox
            * spinImageNum_,
            * spinImageW_,
            * spinImageH_,
            * spinImageFps_;
    QComboBox
            * cbImageFormat_;
    QLabel
            * labelImageName_;
};

RenderDialog::RenderDialog(QWidget *parent)
    : QDialog       (parent)
    , p_            (new Private(this))
{
    setObjectName("_RenderDialog");
    setMinimumSize(640, 640);

    p_->createWidgets();
}

RenderDialog::~RenderDialog()
{
    delete p_;
}


void RenderDialog::Private::createWidgets()
{
    auto lv0 = new QVBoxLayout(diag);

        // output directory

        editDir_ = new FilenameInput(IO::FT_ANY, true, diag);
        editDir_->setFilename(rendSet.directory());
        lv0->addWidget(editDir_);
        connect(editDir_, SIGNAL(filenameChanged(QString)),
                diag, SLOT(p_onWidget_()));

        auto frame = new QFrame(diag);
        frame->setFrameShape(QFrame::HLine);
        lv0->addWidget(frame);


        auto lh = new QHBoxLayout();
        lv0->addLayout(lh);

            // ---- image ----

            auto lv = new QVBoxLayout();
            lh->addLayout(lv);

                editImageName_ = new QLineEdit(diag);
                editImageName_->setText(rendSet.imagePattern());
                lv->addWidget(editImageName_);
                connect(editImageName_, SIGNAL(textChanged(QString)),
                        diag, SLOT(p_onWidget_()));

                spinImageNum_ = new SpinBox(diag);
                spinImageNum_->setLabel(tr("frame number offset"));
                spinImageNum_->setRange(0, 999999999);
                spinImageNum_->setValue(rendSet.imagePatternOffset());
                lv->addWidget(spinImageNum_);
                connect(spinImageNum_, SIGNAL(valueChanged(int)),
                        diag, SLOT(p_onWidget_()));

                labelImageName_ = new QLabel(diag);
                lv->addWidget(labelImageName_);

                spinImageW_ = new SpinBox(diag);
                spinImageW_->setLabel(tr("width"));
                spinImageW_->setRange(0, 4096*4);
                spinImageW_->setValue(rendSet.imageWidth());
                lv->addWidget(spinImageW_);
                connect(spinImageW_, SIGNAL(valueChanged(int)),
                        diag, SLOT(p_onWidget_()));

                spinImageH_ = new SpinBox(diag);
                spinImageH_->setLabel(tr("height"));
                spinImageH_->setRange(0, 4096*4);
                spinImageH_->setValue(rendSet.imageHeight());
                lv->addWidget(spinImageH_);
                connect(spinImageH_, SIGNAL(valueChanged(int)),
                        diag, SLOT(p_onWidget_()));

                spinImageFps_ = new SpinBox(diag);
                spinImageFps_->setLabel(tr("frames per second"));
                spinImageFps_->setRange(0, 60000);
                spinImageFps_->setValue(rendSet.imageFps());
                lv->addWidget(spinImageFps_);
                connect(spinImageFps_, SIGNAL(valueChanged(int)),
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
        connect(but, SIGNAL(pressed()), diag, SLOT(reject()));
        lh->addWidget(but);
}

void RenderDialog::p_onWidget_()
{
    p_->updateFromWidgets();
}

void RenderDialog::Private::updateFromWidgets()
{

}



void RenderDialog::render()
{
    accept();
}



} // namespace GUI
} // namespace MO
