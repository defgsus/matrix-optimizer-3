/** @file qvariantwidget.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#include <functional>

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include "qvariantwidget.h"
#include "spinbox.h"
#include "doublespinbox.h"
#include "coloreditwidget.h"
#include "io/error.h"


namespace MO {
namespace GUI {


struct QVariantWidget::Private
{
    Private(const QString& n, const QVariant& v, QVariantWidget * w)
        : widget(w), v(v), name(n),
          ignore_widget(false),
          l(0), label(0), edit(0) { }

    /** Creates or re-creates the appropriate widgets.
        Can safely be called before or after changing the variant type */
    void createWidgets();
    /** Updates the widget to the current value */
    void updateWidget();

    QVariantWidget * widget;
    QVariant v;
    QString name;

    std::function<void()>
        f_update_widget,
        f_update_value;
    bool ignore_widget;

    QLayout * l;
    QLabel * label;
    QWidget * edit;
};


QVariantWidget::QVariantWidget(const QString& n, const QVariant& v, QWidget *parent)
    : QWidget       (parent),
      p_            (new Private(n, v, this))
{
    p_->createWidgets();
}

QVariantWidget::QVariantWidget(QWidget *parent)
    : QVariantWidget    (QString(), QVariant(), parent)
{

}

const QVariant& QVariantWidget::value() const
{
    return p_->v;
}

void QVariantWidget::Private::updateWidget()
{
    if (f_update_widget)
    {
        // don't emit user signal
        ignore_widget = true;
        // update the widget value
        f_update_widget();
        ignore_widget = false;
    }
}

void QVariantWidget::onValueChange_()
{
    if (p_->ignore_widget)
        return;

    if (p_->f_update_value)
    {
        // copy widget value into own Properties
        p_->f_update_value();
        // emit the user signal
        emit valueChanged();
    }
}

void QVariantWidget::Private::createWidgets()
{
    if (!l)
    {
        l = new QHBoxLayout(widget);
        l->setMargin(2);
    }

    if (!label)
    {
        label = new QLabel(name, widget);
        l->addWidget(label);
    }
    else
        label->setText(name);

    if (edit)
    {
        edit->setVisible(false);
        edit->deleteLater();
        edit = 0;
        f_update_value = 0;
        f_update_widget = 0;
    }

    // create appropriate sub-widgets
    switch (v.type())
    {
        case QVariant::Bool:
        {
            auto cb = new QCheckBox(widget);
            edit = cb;
            f_update_widget = [=](){ cb->setChecked(v.toBool()); };
            f_update_value = [=](){ v = cb->isChecked(); };
            connect(cb, SIGNAL(stateChanged(int)), widget, SLOT(onValueChange_()));
        }
        break;

        case QVariant::Int:
        {
            auto sb = new SpinBox(widget);
            edit = sb;
            sb->setRange(-9999999, 9999999);
            f_update_widget = [=](){ sb->setValue(v.toInt()); };
            f_update_value = [=](){ v = sb->value(); };
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(onValueChange_()));
        }
        break;

        case QVariant::Double:
        {
            auto sb = new DoubleSpinBox(widget);
            edit = sb;
            sb->setRange(-9999999, 9999999);
            f_update_widget = [=](){ sb->setValue(v.toDouble()); };
            f_update_value = [=](){ v = sb->value(); };
            connect(sb, SIGNAL(valueChanged(double)), widget, SLOT(onValueChange_()));
        }
        break;

        case QVariant::String:
        {
            auto e = new QLineEdit(widget);
            edit = e;
            e->setReadOnly(false);
            e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
            f_update_widget = [=](){ e->setText(v.toString()); };
            f_update_value = [=](){ v = e->text(); };
            connect(e, SIGNAL(textChanged(QString)), widget, SLOT(onValueChange_()));
        }
        break;

        case QVariant::Color:
        {
            auto e = new ColorEditWidget(widget);
            edit = e;
            f_update_widget = [=](){ e->setCurrentColor(v.value<QColor>()); };
            f_update_value = [=](){ v = e->currentColor(); };
            connect(e, SIGNAL(textChanged(QString)), widget, SLOT(onValueChange_()));
        }
        break;

        default:
            MO_WARNING("QVariantWidget: unhandled type '" << v.typeName() << "'");
        break;
    }

    if (edit)
    {
        updateWidget();
        l->addWidget(edit);
    }
}


} // namespace GUI
} // namespace MO
