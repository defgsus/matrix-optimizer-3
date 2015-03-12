/** @file serverview.cpp

    @brief Dialog for controlling clients

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>

#include "serverview.h"
#include "widget/netlogwidget.h"
#include "gui/widget/spinbox.h"
#include "engine/serverengine.h"
#include "network/netevent.h"
#include "io/application.h"
#include "io/settings.h"
#include "io/log.h"

namespace MO {
namespace GUI {

ServerView::ServerView(QWidget *parent)
    : QWidget       (parent)
{
    MO_DEBUG_GUI("ServerView::ServerView(" << parent << ")");

    setObjectName("_ServerView");
    //setWindowTitle("Server/client settings");

    setMinimumSize(320,320);
//    setModal(false);

//    settings()->restoreGeometry(this);

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

ServerView::~ServerView()
{
//    settings()->storeGeometry(this);
}

void ServerView::createWidgets_()
{
    const bool isRunning = settings()->value("Server/running").toBool();

    auto lv = new QVBoxLayout(this);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            // --- run state ---

            cbRunning_ = new QCheckBox(tr("run server"), this);
            lh->addWidget(cbRunning_);
            cbRunning_->setChecked(isRunning);
            connect(cbRunning_, SIGNAL(clicked(bool)), this, SLOT(startServer_(bool)));

            labelNum_ = new QLabel(this);
            lh->addWidget(labelNum_);

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            // --- send scene ---

            butSendScene_ = new QPushButton(tr("send current scene"), this);
            butSendScene_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            butSendScene_->setEnabled(isRunning);
            lh->addWidget(butSendScene_);
            connect(butSendScene_, SIGNAL(clicked()), this, SIGNAL(sendScene()));

        // --- clients ---

        clientLayout_ = new QVBoxLayout();
        lv->addLayout(clientLayout_);

        // --- log ---

        // connects itself to NetworkLogger
        logger_ = new NetLogWidget(this);
        lv->addWidget(logger_);
}

void ServerView::onClientsChanged_()
{
    MO_DEBUG("ServerView::onClientsChanged() "
             << server_->numClients());

    updateClientWidgets_();
}

void ServerView::onClientMessage_(const ClientInfo & c, int level, const QString & msg)
{
    logger_->addLine(level, QString("Client[%1]: %2")
                     .arg(c.index)
                     .arg(msg));
}

void ServerView::updateClientWidgets_()
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

QWidget * ServerView::createClientWidget_(int index, const ClientInfo & inf)
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

    return w;
}

void ServerView::startServer_(bool run)
{
    if (run)
        server_->open();
    else
        server_->close();

    // store as default
    settings()->setValue("Server/running", run);

    butSendScene_->setEnabled(server_->isRunning());
    //cbRunning_->setChecked(server_->isRunning());
}

} // namespace GUI
} // namespace MO
