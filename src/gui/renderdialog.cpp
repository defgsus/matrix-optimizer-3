/** @file renderdialog.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/25/2015</p>
*/

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

#include "renderdialog.h"
#include "widget/doublespinbox.h"
#include "widget/spinbox.h"
#include "widget/filenameinput.h"
#include "io/application.h"

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

    RenderDialog * diag;

    FilenameInput * editDir_;
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
    auto lv = new QVBoxLayout(diag);

        // output directory

        editDir_ = new FilenameInput(IO::FT_ANY, true, diag);
        editDir_->setFilename(application()->applicationDirPath());
        lv->addWidget(editDir_);

        auto frame = new QFrame(diag);
        frame->setFrameShape(QFrame::HLine);
        lv->addWidget(frame);

        lv->addStretch(1);


        // [Go!] [Cancel] buttons

        frame = new QFrame(diag);
        frame->setFrameShape(QFrame::HLine);
        lv->addWidget(frame);

        auto lh = new QHBoxLayout(diag);
        lv->addLayout(lh);

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


void RenderDialog::render()
{
    accept();
}


} // namespace GUI
} // namespace MO
