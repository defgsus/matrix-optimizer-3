/** @file propertiesview.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#ifndef PROPERTIESVIEW_H
#define PROPERTIESVIEW_H

#include <QWidget>
#include <QMap>

class QVBoxLayout;
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

    /** Assigns a new set of properties to edit */
    void setProperties(const Properties& );

private:

    void createWidgtes_();

    Properties * p_props_;
    QMap<QString, QVariantWidget*> p_widgets_;
    QVBoxLayout * p_lv_;
    QWidget * p_stretch_, * p_container_;
    QScrollArea * p_scroll_;
};

} // namespace GUI
} // namespace MO


#endif // PROPERTIESVIEW_H
