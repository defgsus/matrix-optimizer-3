/** @file networkmanager.cpp

    @brief Basic wrapper around Qt's network stuff

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#include <sstream>
#include <iomanip>

#include <QString>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtNetwork/QNetworkConfiguration>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkSession>
#include <QtNetwork/QTcpServer>

#include "networkmanager.h"

namespace MO {

NetworkManager::NetworkManager(QObject *parent)
    : QObject   (parent),
      conf_     (new QNetworkConfigurationManager(this)),
      //mgr_    (new QNetworkAccessManager(this)),
      net_      (0)
{

}




QString NetworkManager::systemInfo() const
{
    const int
        BEARER_WIDTH = 10,
        NAME_WIDTH = 25,
        PURPOSE_WIDTH = 12,
        STATE_WIDTH = 12,
        TYPE_WIDTH = 10;

    std::stringstream s;

    s << std::left;

    // header
    s << std::setw(BEARER_WIDTH) << "BEARER"
      << std::setw(NAME_WIDTH) << "NAME"
      << std::setw(PURPOSE_WIDTH) << "PURPOSE"
      << std::setw(TYPE_WIDTH) << "TYPE"
      << std::setw(STATE_WIDTH) << "STATE"
      << "\n";

    // each found network
    for (auto &i : conf_->allConfigurations())
    {
        s
          << std::setw(BEARER_WIDTH) << i.bearerTypeName().toStdString()
          << std::setw(NAME_WIDTH) << i.name().toStdString();

        s << std::setw(PURPOSE_WIDTH);
        switch (i.purpose())
        {
            case QNetworkConfiguration::PublicPurpose: s << "public"; break;
            case QNetworkConfiguration::PrivatePurpose: s << "private"; break;
            case QNetworkConfiguration::ServiceSpecificPurpose: s << "service"; break;
            default: s << "unknown";
        }

        s << std::setw(TYPE_WIDTH);
        switch (i.type())
        {
            case QNetworkConfiguration::InternetAccessPoint: s << "internet"; break;
            case QNetworkConfiguration::ServiceNetwork: s << "service"; break;
            case QNetworkConfiguration::UserChoice: s << "user"; break;
            default: s << "invalid";
        }

        s << std::setw(STATE_WIDTH);
        switch (i.state())
        {
            case QNetworkConfiguration::Defined: s << "defined"; break;
            case QNetworkConfiguration::Discovered: s << "discovered"; break;
            case QNetworkConfiguration::Active: s << "active"; break;
            default: s << "undefined";
        }

        s << "\n";
    }
    return QString::fromStdString(s.str());
}

QString NetworkManager::networkInfo() const
{
    if (!net_) return QString();

    const int
        BROADCAST_WIDTH = 17,
        NETMASK_WIDTH = 17,
        IP_WIDTH = 40;

    std::stringstream s;
    s << std::left;

    QNetworkInterface face = net_->interface();

    s << face.humanReadableName().toStdString()
      << " (" << face.name().toStdString() << ")" << "\n";

    // header
    s << std::setw(BROADCAST_WIDTH) << "BROADCAST"
      << std::setw(NETMASK_WIDTH) << "NETMASK"
      << std::setw(IP_WIDTH) << "IP"
      << "\n";

    // list
    auto list = face.addressEntries();
    for (auto &i : list)
    {
        //QNetworkAddressEntry::broadcast()
        s << std::setw(BROADCAST_WIDTH) << i.broadcast().toString().toStdString()
          << std::setw(NETMASK_WIDTH) << i.netmask().toString().toStdString()
          << std::setw(IP_WIDTH) << i.ip().toString().toStdString();
    }

    s << "\n";

    return QString::fromStdString(s.str());
}


QNetworkConfiguration * NetworkManager::defaultNetwork() const
{
    for (auto &i : conf_->allConfigurations())
    {
        if (i.bearerType() == QNetworkConfiguration::BearerEthernet
                &&  i.state() == QNetworkConfiguration::Active)
        {
            return &i;
        }
    }

    return 0;
}


void NetworkManager::open()
{
    auto c = defaultNetwork();
    if (!c) return;

    if (net_)
        net_->deleteLater();

    net_ = new QNetworkSession(*c, this);
    connect(net_, SIGNAL(opened()), this, SLOT(slotOpened_()));
    connect(net_, SIGNAL(error(QNetworkSession::SessionError)),
            this, SLOT(slotError_(QNetworkSession::SessionError)));
    connect(net_, SIGNAL(closed()), this, SLOT(slotClosed_()));

    net_->open();
}

void NetworkManager::close()
{
    if (!net_)
        return;

    net_->close();
}

bool NetworkManager::isOpen() const
{
    return (net_ && net_->isOpen());
}

void NetworkManager::slotOpened_()
{
    emit opened();
}

void NetworkManager::slotClosed_()
{
    emit closed();
}

void NetworkManager::slotError_(QNetworkSession::SessionError e)
{
    QString s;
    switch (e)
    {
        case QNetworkSession::UnknownSessionError: s = "unknown session error"; break;
        case QNetworkSession::SessionAbortedError: s = "session was aborted"; break;
        case QNetworkSession::RoamingError: s = "roaming error"; break;
        case QNetworkSession::OperationNotSupportedError: s = "operation not supported"; break;
        case QNetworkSession::InvalidConfigurationError: s = "invalid configuration"; break;
    }

    emit error(s);
}

} // namespace MO
