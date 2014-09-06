/** @file networkdialog.cpp

    @brief Network config dialog

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#include <QTableWidget>
#include <QLayout>
#include <QPushButton>
#include <QNetworkConfigurationManager>
#include <QNetworkConfiguration>
#include <QNetworkInterface>

#include "networkdialog.h"
#include "io/log.h"

namespace MO {
namespace GUI {

QString NetworkDialog::purposeName(int purpose)
{
    switch (purpose)
    {
        case QNetworkConfiguration::PublicPurpose: return tr("public");
        case QNetworkConfiguration::PrivatePurpose: return tr("private");
        case QNetworkConfiguration::ServiceSpecificPurpose: return tr("service");
        default: return tr("unknown");
    }
}

QString NetworkDialog::typeName(int type)
{
    switch (type)
    {
        case QNetworkConfiguration::InternetAccessPoint: return tr("internet");
        case QNetworkConfiguration::ServiceNetwork: return tr("service");
        case QNetworkConfiguration::UserChoice: return tr("user");
        default: return tr("invalid");
    }
}

QString NetworkDialog::stateName(int state)
{
    QString str;
    if (state & QNetworkConfiguration::Defined)
        str += tr("defined");
    if (state & QNetworkConfiguration::Discovered)
        str += tr(" discovered");
    if (state & QNetworkConfiguration::Active)
        str += tr(" active");
    return str;
}

NetworkDialog::NetworkDialog(QWidget *parent) :
    QDialog(parent)
{
    setObjectName("_NetworkDialog");
    setWindowTitle(tr("Network configuration"));

    setMinimumSize(640,480);

    createWidgets_();

    rescan();
}

void NetworkDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        table_ = new QTableWidget(this);
        lv->addWidget(table_);

        table_->setColumnCount(5);
        table_->setColumnWidth(0, 200);
        table_->setSortingEnabled(true);

}

void NetworkDialog::rescan()
{
    table_->clear();
    table_->setHorizontalHeaderLabels(
                QStringList()
                << tr("name")
                << tr("bearer")
                << tr("purpose")
                << tr("type")
                << tr("state"));

    // get all available networks
    QNetworkConfigurationManager mgr;
    auto list = mgr.allConfigurations();

    // --- fill table ---

    table_->setRowCount(list.count());

    int row = 0;
    for (const QNetworkConfiguration& i : list)
    {
        // -- name --

        auto item = new QTableWidgetItem();
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setText(i.name());

        table_->setItem(row, 0, item);

        // -- bearer --

        item = new QTableWidgetItem();
        item->setFlags(Qt::ItemIsEnabled);
        item->setText(i.bearerTypeName());

        table_->setItem(row, 1, item);

        // -- purpose --

        item = new QTableWidgetItem();
        item->setFlags(Qt::ItemIsEnabled);
        item->setText(purposeName(i.purpose()));

        table_->setItem(row, 2, item);

        // -- type --

        item = new QTableWidgetItem();
        item->setFlags(Qt::ItemIsEnabled);
        item->setText(typeName(i.type()));

        table_->setItem(row, 3, item);

        // -- state --

        item = new QTableWidgetItem();
        item->setFlags(Qt::ItemIsEnabled);
        item->setText(stateName(i.state()));

        table_->setItem(row, 4, item);

        ++row;
    }
}

} // namespace GUI
} // namespace MO
