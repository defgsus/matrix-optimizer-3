/** @file serverdialog.cpp

    @brief Dialog for controlling clients

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>

#include "serverdialog.h"
#include "io/application.h"
#include "engine/serverengine.h"
#include "network/netevent.h"
#include "widget/netlogwidget.h"
#include "io/settings.h"

namespace MO {
namespace GUI {


ServerDialog::ServerDialog(QWidget *parent) :
    QDialog(parent)
{
    setObjectName("_ServerDialog");
    setWindowTitle("Server/client settings");

    setMinimumSize(640,640);
    setModal(false);

    settings->restoreGeometry(this);

    server_ = application->serverEngine();

    createWidgets_();
}

ServerDialog::~ServerDialog()
{
    settings->saveGeometry(this);
}

void ServerDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        // --- run state ---

        cbRunning_ = new QCheckBox(tr("run server"), this);
        lv->addWidget(cbRunning_);
        cbRunning_->setChecked(server_->isRunning());
        connect(cbRunning_, SIGNAL(clicked(bool)), this, SLOT(startServer_(bool)));



        // --- log ---

        auto logger = new NetLogWidget(this);
        lv->addWidget(logger);
}

void ServerDialog::startServer_(bool run)
{
    if (run)
        server_->open();
    else
        server_->close();

    //cbRunning_->setChecked(server_->isRunning());
}

} // namespace GUI
} // namespace MO
