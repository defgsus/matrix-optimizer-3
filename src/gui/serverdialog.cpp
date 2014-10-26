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
#include "gui/widget/spinbox.h"

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
    connect(server_, SIGNAL(clientStateChanged(int)),
            this, SLOT(updateClientWidgets_()));
    connect(server_, SIGNAL(clientMessage(ClientInfo,int,QString)),
            this, SLOT(onClientMessage_(ClientInfo,int,QString)));
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

        // connects itself to NetworkLogger
        logger_ = new NetLogWidget(this);
        lv->addWidget(logger_);
}

void ServerDialog::onClientsChanged_()
{
    MO_DEBUG("ServerDialog::onClientsChanged() "
             << server_->numClients());

    updateClientWidgets_();
}

void ServerDialog::onClientMessage_(const ClientInfo & c, int level, const QString & msg)
{
    logger_->addLine(level, QString("Client[%1]: %2")
                     .arg(c.index)
                     .arg(msg));
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

        auto label = new QLabel(tr("Client %1 (desktop %2)\n"
                                   "info win %3, render win %4, files ready %5, scene ready %6, playing %7")
                                .arg(inf.index)
                                .arg(inf.state.desktop())
                                .arg(inf.state.isInfoWindow() ? tr("yes") : tr("no"))
                                .arg(inf.state.isRenderWindow() ? tr("yes") : tr("no"))
                                .arg(inf.state.isFilesReady() ? tr("yes") : tr("no"))
                                .arg(inf.state.isSceneReady() ? tr("yes") : tr("no"))
                                .arg(inf.state.isPlayback() ? tr("yes") : tr("no"))
                                , w);
        lv->addWidget(label);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            label = new QLabel(tr("client index"), w);
            label->setAlignment(Qt::AlignRight);
            lh->addWidget(label);

            auto sb = new SpinBox(w);
            lh->addWidget(sb);
            sb->setMinimum(0);
            sb->setValue(inf.index);
            connect(sb, &SpinBox::valueChanged, [=](int val)
            {
                server_->setClientIndex(index, val);
            });

            label = new QLabel(tr("output screen"), w);
            label->setAlignment(Qt::AlignRight);
            lh->addWidget(label);

            sb = new SpinBox(w);
            lh->addWidget(sb);
            sb->setRange(0, inf.sysinfo.numScreens() - 1);
            sb->setValue(inf.state.desktop());
            connect(sb, &SpinBox::valueChanged, [=](int val)
            {
                server_->setDesktopIndex(index, val);
            });

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto cb = new QCheckBox(tr("show info window"), w);
            lh->addWidget(cb);
            cb->setChecked(inf.state.isInfoWindow());
            connect(cb, &QCheckBox::clicked, [=](bool s)
            {
                server_->showInfoWindow(index, s);
            });

            cb = new QCheckBox(tr("show render window"), w);
            lh->addWidget(cb);
            cb->setChecked(inf.state.isRenderWindow());
            connect(cb, &QCheckBox::clicked, [=](bool s)
            {
                server_->showRenderWindow(index, s);
            });

            auto but = new QPushButton(tr("send scene"), w);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked()), this, SIGNAL(sendScene()));

    return w;
}

void ServerDialog::startServer_(bool run)
{
    if (run)
        server_->open();
    else
        server_->close();

    // store as default
    settings->setValue("Server/running", run);


    //cbRunning_->setChecked(server_->isRunning());
}

} // namespace GUI
} // namespace MO
