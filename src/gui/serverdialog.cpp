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
#include "network/tcpserver.h"
#include "network/netevent.h"

namespace MO {
namespace GUI {


ServerDialog::ServerDialog(QWidget *parent) :
    QDialog(parent)
{
    setObjectName("_ServerDialog");
    setWindowTitle("Server/client settings");

    setMinimumSize(640,480);

    server_ = application->server();

    createWidgets_();
}

void ServerDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        cbRunning_ = new QCheckBox(tr("run server"), this);
        lv->addWidget(cbRunning_);
        cbRunning_->setChecked(server_->isListening());
        connect(cbRunning_, SIGNAL(clicked(bool)), this, SLOT(startServer_(bool)));
}

void ServerDialog::startServer_(bool run)
{
    if (run)
        server_->open();
    else
        server_->close();

    cbRunning_->setChecked(server_->isListening());
}

} // namespace GUI
} // namespace MO
