/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10/20/2015</p>
*/

#include <QListWidget>
#include <QLayout>

#include "oscview.h"
#include "network/oscinput.h"
#include "network/oscinputs.h"

namespace MO {
namespace GUI {

struct OscView::Private
{
    Private(OscView * w)
        : widget(w)
    {

    }

    void createWidgets();
    void updateListeners();
    void updateValues();

    OscView * widget;
    QListWidget * listPort, * listValue;
};

OscView::OscView(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    setObjectName("_OscView");

    p_->createWidgets();

    connect(OscInputs::instance(), SIGNAL(listenersChanged()),
            this, SLOT(onListenersChanged()), Qt::QueuedConnection);
}

OscView::~OscView()
{
    delete p_;
}

void OscView::Private::createWidgets()
{
    auto lv = new QVBoxLayout(widget);

        listPort = new QListWidget(widget);
        lv->addWidget(listPort);
        connect(listPort, SIGNAL(currentRowChanged(int)),
                widget, SLOT(onValuesChanged()));

        listValue = new QListWidget(widget);
        lv->addWidget(listValue);
}

void OscView::onListenersChanged()
{
    p_->updateListeners();
}

void OscView::onValuesChanged()
{
    p_->updateValues();
}

void OscView::showEvent(QShowEvent * e)
{
    QWidget::showEvent(e);
    p_->updateValues();
}

void OscView::Private::updateListeners()
{
    widget->setUpdatesEnabled(false);

    const auto oscs = OscInputs::listeners();

    listPort->clear();
    for (OscInput * osc : oscs)
    {
        auto item = new QListWidgetItem(
                            QString("%1").arg(osc->port()), listPort);
        item->setData(Qt::UserRole, qlonglong(osc));
        listPort->addItem(item);

        connect(osc, SIGNAL(valueChanged(QString)),
                widget, SLOT(onValuesChanged()),
                Qt::ConnectionType(Qt::QueuedConnection | Qt::UniqueConnection));
    }

    widget->setUpdatesEnabled(true);
}

void OscView::Private::updateValues()
{
    if (!widget->isVisible())
        return;

    auto item = listPort->currentItem();
    if (!item)
        return;

    OscInput * inp = (OscInput*)item->data(Qt::UserRole).toLongLong();

    widget->setUpdatesEnabled(false);

    listValue->clear();
    for (auto i = inp->values().begin(); i != inp->values().end(); ++i)
        listValue->addItem(QString("%1 %2").arg(i.key()).arg(i.value().toString()));

    widget->setUpdatesEnabled(true);
}

} // namespace GUI
} // namespace MO
