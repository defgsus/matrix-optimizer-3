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
#include <QTabWidget>
#include <QTextStream>

#include "serverview.h"
#include "widget/netlogwidget.h"
#include "gui/widget/spinbox.h"
#include "engine/serverengine.h"
#include "network/netevent.h"
#include "tool/stringmanip.h"
#include "io/application.h"
#include "io/settings.h"
#include "io/log.h"

namespace MO {
namespace GUI {

ServerView::ServerView(QWidget *parent)
    : QWidget       (parent)
    , tabWidget_    (0)
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
        tabWidget_ = 0;

        // --- log ---

        // connects itself to NetworkLogger
        logger_ = new NetLogWidget(this);
        lv->addWidget(logger_, 1);
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
    setUpdatesEnabled(false);

    // delete previous
    /*
    for (auto w : clientWidgets_)
    {
        w->setVisible(false);
        w->deleteLater();
    }
    clientWidgets_.clear();
    */

    int index = -1;

    // XXX When removing via QTabWidget::removeTab()
    // the tab still stays visible, so we recreate the whole thing
    if (tabWidget_)
    {
        index = tabWidget_->currentIndex();
        tabWidget_->deleteLater();
    }
    tabWidget_ = new QTabWidget(this);
    clientLayout_->addWidget(tabWidget_);

    // update other widgets
    labelNum_->setText(tr("%1 connected clients").arg(server_->numClients()));

    // create client widgets
    for (int i=0; i<server_->numClients(); ++i)
    {
        QWidget * w = createClientWidget_(i, server_->clientInfo(i));
        clientWidgets_.append(w);
        tabWidget_->addTab(w, QString("%1").arg(i+1));
    }

    // restore selected tab
    if (index >= 0 && index < tabWidget_->count())
        tabWidget_->setCurrentIndex(index);

    setUpdatesEnabled(true);
}

QWidget * ServerView::createClientWidget_(int index, const ClientInfo & inf)
{
    QWidget * w = new QWidget(tabWidget_);
    auto lv = new QVBoxLayout(w);

        QString labelText;
        QTextStream str(&labelText);
        str << "Client " << inf.index << " (desktop " << inf.state.desktop()
            << " / res = " << inf.state.outputSize().width() << "x" << inf.state.outputSize().height() << ")"
            << "\ninfo win " << (inf.state.isInfoWindow() ? tr("YES") : tr("no"))
            << " render win " << (inf.state.isRenderWindow() ? tr("YES") : tr("no"))
            << " files ready " << (inf.state.isFilesReady() ? tr("YES") : tr("no"))
            << " playing " << (inf.state.isPlayback() ? tr("YES") : tr("no"))
            << "\nmemory use " << byte_to_string(inf.state.memory())
            << ", file cache " << byte_to_string(inf.state.cacheSize());

        auto label = new QLabel(labelText, w);
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

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QPushButton(tr("clear file cache"), w);
            lh->addWidget(but);
            connect(but, &QPushButton::clicked, [=]()
            {
                server_->sendClearFileCache(index);
            });

            but = new QPushButton(tr("update state"), w);
            lh->addWidget(but);
            connect(but, &QPushButton::clicked, [=]()
            {
                server_->getClientState(index);
            });

            lh->addStretch(1);

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
