/** @file qvariantwidget.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#ifndef QVARIANTWIDGET_H
#define QVARIANTWIDGET_H

#include <QWidget>

namespace MO {
class Properties;
namespace GUI {

/** Generic editor for a QVariant.
    Most common types supported.
    It also handles stuff specific to MO::Properties. */
class QVariantWidget : public QWidget
{
    Q_OBJECT
public:

    explicit QVariantWidget(QWidget *parent = 0);
    explicit QVariantWidget(const QString& n, const QVariant& v, QWidget *parent = 0);
    /** Constructor for a property (more than a QVariant) */
    explicit QVariantWidget(const QString& id, const Properties* p, QWidget *parent = 0);

    /** Returns the currently set value */
    const QVariant& value() const;

signals:

    /** Emitted when the user changed the value (and only then). */
    void valueChanged();

public slots:

private slots:

    void onValueChanged_();

private:

    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // QVARIANTWIDGET_H
