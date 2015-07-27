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
#include <QComboBox>
#include <QToolButton>

#include "qvariantwidget.h"
#include "spinbox.h"
#include "doublespinbox.h"
#include "coloreditwidget.h"
#include "gui/texteditdialog.h"
#include "types/properties.h"
#include "object/object_fwd.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {


struct QVariantWidget::Private
{
    Private(const QString& n, const QVariant& v, QVariantWidget * w)
        : widget(w), props(0), v(v), name(n),
          ignore_widget(false),
          layout(0), label(0), edit(0) { }

    /** Creates or re-creates the appropriate widgets.
        Can safely be called before or after changing the variant type */
    void createWidgets();
    /** Updates the widget to the current value */
    void updateWidget();

    QVariantWidget * widget;
    const Properties * props;
    QString id, tip;
    QVariant v;
    QString name;

    std::function<void()>
        f_update_widget,
        f_update_value;
    bool ignore_widget;

    QHBoxLayout * layout;
    QLabel * label;
    QWidget * edit;
};

QVariantWidget::QVariantWidget(const QString& id, const Properties * prop, QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(QString(), prop->get(id), this))
{
    p_->id = id;
    p_->props = prop;
    p_->name = prop->hasName(id) ? prop->getName(id) : id;
    p_->tip = prop->getTip(id);
    p_->createWidgets();
}

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

void QVariantWidget::onValueChanged_()
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
    if (!layout)
    {
        layout = new QHBoxLayout(widget);
        layout->setMargin(2);
        //l->setSizeConstraint(QLayout::SetMaximumSize);
    }

    if (!label)
    {
        label = new QLabel(name, widget);
        layout->addWidget(label);
    }
    else
        label->setText(name);

    //layout->addStretch(1);

    // clear previous edit widget
    if (edit)
    {
        edit->setVisible(false);
        edit->deleteLater();
        edit = 0;
        f_update_value = 0;
        f_update_widget = 0;
    }

    widget->setStatusTip(tip);


    // ----- create appropriate sub-widgets -----

    //MO_DEBUG("qvariantwidget '" << name << "': type '" << v.typeName() << "' (" << v.type() << ")");

#define MO__SUBLAYOUT(Layout__) \
    edit = new QWidget(widget); \
    auto layout = new Layout__(edit); \
    layout->setMargin(0);

    bool isHandled = true;
    switch ((int)v.type())
    {
        case QMetaType::Bool:
        {
            auto cb = new QCheckBox(widget);
            edit = cb;
            f_update_widget = [=](){ cb->setChecked(v.toBool()); };
            f_update_value = [=](){ v = cb->isChecked(); };
            connect(cb, SIGNAL(stateChanged(int)), widget, SLOT(onValueChanged_()));
        }
        break;

        case QMetaType::Int:
        case QMetaType::UInt:
        {
            bool hasSign = v.type() == QVariant::UInt;
            auto sb = new SpinBox(widget);
            edit = sb;
            sb->setRange(hasSign ? -999999999 : 0, 999999999);
            if (props)
            {
                if (props->hasMin(id))
                    sb->setMinimum(props->getMin(id).toInt());
                if (props->hasMax(id))
                    sb->setMaximum(props->getMax(id).toInt());
                if (props->hasStep(id))
                    sb->setSingleStep(props->getStep(id).toInt());
            }
            f_update_widget = [=](){ sb->setValue(v.toInt()); };
            if (hasSign)
                f_update_value = [=](){ v = sb->value(); };
            else
                f_update_value = [=](){ v = (unsigned)sb->value(); };
            connect(sb, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
        }
        break;

        case QMetaType::Double:
        case QMetaType::Float:
        {
            auto sb = new DoubleSpinBox(widget);
            edit = sb;
            //sb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
            sb->setRange(-9999999, 9999999);
            sb->setDecimals(7);
            if (props)
            {
                if (props->hasMin(id))
                    sb->setMinimum(props->getMin(id).toDouble());
                if (props->hasMax(id))
                    sb->setMaximum(props->getMax(id).toDouble());
                if (props->hasStep(id))
                    sb->setSingleStep(props->getStep(id).toDouble());
            }
            f_update_widget = [=](){ sb->setValue(v.toDouble()); };
            if (v.type() == QVariant::Double)
                f_update_value = [=](){ v = double(sb->value()); };
            else
                f_update_value = [=](){ v = float(sb->value()); };
            connect(sb, SIGNAL(valueChanged(double)), widget, SLOT(onValueChanged_()));
        }
        break;

        case QMetaType::QString:
        {
            int subtype = props && props->hasSubType(id)
                    ? props->getSubType(id)
                    : -1;
            // string display with edit button (->TextEditDialog)
            if (subtype != -1)
            {
                MO__SUBLAYOUT(QHBoxLayout);
                auto e = new QLineEdit(widget);
                e->setReadOnly(true);
                e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                layout->addWidget(e);

                auto b = new QToolButton(widget);
                b->setText(tr("..."));
                layout->addWidget(b);
                connect(b, &QToolButton::clicked, [=]()
                {
                    /** @todo script engines need to be passed to dialog */
                    auto diag = new TextEditDialog(TextType(subtype), widget);
                    diag->setAttribute(Qt::WA_DeleteOnClose);
                    diag->setText(v.toString());
                    connect(diag, &TextEditDialog::textChanged, [=]()
                    {
                        e->setText(diag->getText());
                        widget->onValueChanged_();
                    });
                    diag->show();
                });
                f_update_widget = [=](){ e->setText(v.toString()); /** @todo missing update of dialog */ };
                f_update_value = [=](){ v = e->text(); };

            }
            // generic string edit
            else
            {
                auto e = new QLineEdit(widget);
                edit = e;
                e->setReadOnly(false);
                e->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
                f_update_widget = [=](){ e->setText(v.toString()); };
                f_update_value = [=](){ v = e->text(); };
                connect(e, SIGNAL(textChanged(QString)), widget, SLOT(onValueChanged_()));
            }
        }
        break;

        case QMetaType::QSize:
        {
            MO__SUBLAYOUT(QHBoxLayout);
            auto sb1 = new SpinBox(widget),
                 sb2 = new SpinBox(widget);
            sb1->setRange(0, 9999999);
            sb2->setRange(0, 9999999);
            if (props)
            {
                if (props->hasMin(id))
                {
                    sb1->setMinimum(props->getMin(id).toSize().width());
                    sb2->setMinimum(props->getMin(id).toSize().height());
                }
                if (props->hasMax(id))
                {
                    sb1->setMaximum(props->getMax(id).toSize().width());
                    sb2->setMaximum(props->getMax(id).toSize().height());
                }
                if (props->hasStep(id))
                {
                    sb1->setSingleStep(props->getStep(id).toSize().width());
                    sb2->setSingleStep(props->getStep(id).toSize().height());
                }
            }
            layout->addWidget(sb1);
            layout->addWidget(sb2);
            f_update_widget = [=](){ auto s = v.toSize(); sb1->setValue(s.width()); sb2->setValue(s.height()); };
            f_update_value = [=](){ v = QSize(sb1->value(), sb2->value()); };
            connect(sb1, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
            connect(sb2, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
        }
        break;

        case QMetaType::QSizeF:
        {
            MO__SUBLAYOUT(QHBoxLayout);
            auto sb1 = new DoubleSpinBox(widget),
                 sb2 = new DoubleSpinBox(widget);
            sb1->setRange(0, 9999999); sb1->setDecimals(4);
            sb2->setRange(0, 9999999); sb2->setDecimals(4);
            if (props)
            {
                if (props->hasMin(id))
                {
                    sb1->setMinimum(props->getMin(id).toSizeF().width());
                    sb2->setMinimum(props->getMin(id).toSizeF().height());
                }
                if (props->hasMax(id))
                {
                    sb1->setMaximum(props->getMax(id).toSizeF().width());
                    sb2->setMaximum(props->getMax(id).toSizeF().height());
                }
                if (props->hasStep(id))
                {
                    sb1->setSingleStep(props->getStep(id).toSizeF().width());
                    sb2->setSingleStep(props->getStep(id).toSizeF().height());
                }
            }
            layout->addWidget(sb1);
            layout->addWidget(sb2);
            f_update_widget = [=](){ auto s = v.toSize(); sb1->setValue(s.width()); sb2->setValue(s.height()); };
            f_update_value = [=](){ v = QSizeF(sb1->value(), sb2->value()); };
            connect(sb1, SIGNAL(valueChanged(double)), widget, SLOT(onValueChanged_()));
            connect(sb2, SIGNAL(valueChanged(double)), widget, SLOT(onValueChanged_()));
        }
        break;

        case QMetaType::QPoint:
        {
            MO__SUBLAYOUT(QHBoxLayout);
            auto sb1 = new SpinBox(widget),
                 sb2 = new SpinBox(widget);
            sb1->setRange(-9999999, 9999999);
            sb2->setRange(-9999999, 9999999);
            if (props)
            {
                if (props->hasMin(id))
                {
                    sb1->setMinimum(props->getMin(id).toPoint().x());
                    sb2->setMinimum(props->getMin(id).toPoint().y());
                }
                if (props->hasMax(id))
                {
                    sb1->setMaximum(props->getMax(id).toPoint().x());
                    sb2->setMaximum(props->getMax(id).toPoint().y());
                }
                if (props->hasStep(id))
                {
                    sb1->setSingleStep(props->getStep(id).toPoint().x());
                    sb2->setSingleStep(props->getStep(id).toPoint().y());
                }
            }
            layout->addWidget(sb1);
            layout->addWidget(sb2);
            f_update_widget = [=](){ auto s = v.toPoint(); sb1->setValue(s.x()); sb2->setValue(s.y()); };
            f_update_value = [=](){ v = QPoint(sb1->value(), sb2->value()); };
            connect(sb1, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
            connect(sb2, SIGNAL(valueChanged(int)), widget, SLOT(onValueChanged_()));
        }
        break;

        case QMetaType::QColor:
        {
            auto e = new ColorEditWidget(widget);
            edit = e;
            f_update_widget = [=](){ e->setCurrentColor(v.value<QColor>()); };
            f_update_value = [=](){ v = e->currentColor(); };
            connect(e, SIGNAL(textChanged(QString)), widget, SLOT(onValueChanged_()));
        }
        break;

        default:
            isHandled = false;
        break;
    }

    // Following are QMetaTypes that are runtime constants
    // and can't be used in switch(), e.g. stuff in Q_DECLARE_METATYPE()
    if (!isHandled)
    {
        /*
        // Generic approach for Properties::NamedStates
        if (auto ns = Properties::getNamedStates(v))
        {
            auto cb = new QComboBox(widget);
            edit = cb;
            for (auto & i : *ns)
                cb->addItem(ns->id(i), i.value().value());
            f_update_widget = [=](){ cb->setCurrentText(ns->id(v)); };
            f_update_value = [=](){ v = cb->itemData(cb->currentIndex()); };
            connect(cb, SIGNAL(currentIndexChanged(int)), widget, SLOT(onValueChanged_()));
            isHandled = true;
        }
        */
    }

    if (edit)
    {
        MO_ASSERT(f_update_widget, "No widget update defined for type '" << v.typeName() << "'");
        MO_ASSERT(f_update_value, "No value update defined for type '" << v.typeName() << "'");
        updateWidget();
        layout->addWidget(edit);
    }

    if (!isHandled)
    {
        MO_WARNING("QVariantWidget: unhandled type '" << v.typeName() << "'");
    }

#undef MO__SUBLAYOUT
}


} // namespace GUI
} // namespace MO
