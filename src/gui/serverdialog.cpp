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
#include "io/log.h"

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

    server_ = &serverEngine();

    createWidgets_();

    updateClientWidgets_();

    connect(server_, SIGNAL(numberClientsChanged(int)),
            this, SLOT(onClientsChanged_()));
}

ServerDialog::~ServerDialog()
{
    settings->saveGeometry(this);
}

void ServerDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            // --- run state ---

            cbRunning_ = new QCheckBox(tr("run server"), this);
            lh->addWidget(cbRunning_);
            cbRunning_->setChecked(server_->isRunning());
            connect(cbRunning_, SIGNAL(clicked(bool)), this, SLOT(startServer_(bool)));

            labelNum_ = new QLabel(this);
            lh->addWidget(labelNum_);

        // --- clients ---

        clientLayout_ = new QVBoxLayout();
        lv->addLayout(clientLayout_);

        // --- log ---

        auto logger = new NetLogWidget(this);
        lv->addWidget(logger);
}

void ServerDialog::onClientsChanged_()
{
    MO_DEBUG("ServerDialog::onClientsChanged() "
             << server_->numClients());

    updateClientWidgets_();
}

void ServerDialog::updateClientWidgets_()
{
    // delete previous
    for (auto w : clientWidgets_)
    {
        w->setVisible(false);
        w->deleteLater();
    }
    clientWidgets_.clear();

    // update other widgets
    labelNum_->setText(tr("%1 connected clients").arg(server_->numClients()));

    // create client widgets
    for (int i=0; i<server_->numClients(); ++i)
    {
        QWidget * w = createClientWidget_(i, server_->clientInfo(i));
        clientWidgets_.append(w);
        clientLayout_->addWidget(w);
    }
}

QWidget * ServerDialog::createClientWidget_(int index, const ClientInfo & inf)
{
    QWidget * w = new QWidget(this);
    auto lv = new QVBoxLayout(w);

        auto label = new QLabel(tr("Client %1").arg(inf.index), w);
        lv->addWidget(label);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QPushButton(tr("show info window"), w);
            lh->addWidget(but);
            connect(but, &QPushButton::clicked, [=]()
            {
                server_->showInfoWindow(index, true);
            });

            but = new QPushButton(tr("hide info window"), w);
            lh->addWidget(but);
            connect(but, &QPushButton::clicked, [=]()
            {
                server_->showInfoWindow(index, false);
            });

    return w;
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
