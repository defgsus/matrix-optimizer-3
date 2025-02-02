/** @file propertiesscrolview.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#ifndef MOSRC_GUI_PROPERTIESCROLLSVIEW_H
#define MOSRC_GUI_PROPERTIESCROLLSVIEW_H

#include <QScrollArea>
#include <QMap>

class QVBoxLayout;
class QScrollArea;

namespace MO {
class Properties;
namespace GUI {
class QVariantWidget;

/** Generic gui display/editor for MO::Properties (types/properties.h) */
class PropertiesScrollView : public QScrollArea
{
    Q_OBJECT
public:
    explicit PropertiesScrollView(QWidget *parent = 0);
    ~PropertiesScrollView();

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
    void updateWidgetVis_();

    Properties * p_props_;
    QMap<QString, QVariantWidget*> p_widgets_;
    QVBoxLayout * p_lv_;
    QWidget * p_stretch_, * p_container_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_PROPERTIESCROLLSVIEW_H
