/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/24/2015</p>
*/

#ifndef MOSRC_GUI_PROPERTIESVIEW_H
#define MOSRC_GUI_PROPERTIESVIEW_H

#include <QWidget>
#include <QMap>

class QGridLayout;
class QScrollArea;

namespace MO {
class Properties;
namespace GUI {
class QVariantWidget;

/** Generic gui display/editor for MO::Properties (types/properties.h) */
class PropertiesView : public QWidget
{
    Q_OBJECT
public:
    explicit PropertiesView(QWidget *parent = 0);
    ~PropertiesView();

    /** Returns read access to the assigned properties */
    const Properties& properties() const { return *p_props_; }

signals:

    /** Emitted when the used has changed a property value */
    void propertyChanged(const QString& id);

public slots:

    /** Destroys all property widgets */
    void clear();

    /** Assigns a new set of Properties to edit */
    void setProperties(const Properties& );

    /** Assigns multiple sets of Properties to edit.
        A widget is created for each unique id in all Properties. */
    void setProperties(const QList<Properties>& );

private:

    void createWidgtes_();

    Properties * p_props_;
    QMap<QString, QVariantWidget*> p_widgets_;
    QGridLayout * p_lg_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_PROPERTIESVIEW_H
