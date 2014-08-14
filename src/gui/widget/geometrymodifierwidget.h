/** @file geometrymodifierwidget.h

    @brief Widget for GeometryModifier classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_GEOMETRYMODIFIERWIDGET_H
#define MOSRC_GUI_WIDGET_GEOMETRYMODIFIERWIDGET_H

#include <functional>

#include <QWidget>

namespace MO {
namespace GEOM { class GeometryModifier; }
namespace GUI {

class GroupWidget;

class GeometryModifierWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GeometryModifierWidget(GEOM::GeometryModifier *, bool expanded, QWidget *parent = 0);

    GEOM::GeometryModifier * geometryModifier() const { return modifier_; }

signals:

    void requestUp(GEOM::GeometryModifier *);
    void requestDown(GEOM::GeometryModifier *);
    void requestDelete(GEOM::GeometryModifier *);
    void requestInsertNew(GEOM::GeometryModifier *);

    void expandedChange(GEOM::GeometryModifier*, bool expanded);

public slots:

    void updateWidgetValues();

private slots:

    void updateFromWidgets_();

private:

    void createWidgets_(bool expanded);
    void updateWidgets_();

    GEOM::GeometryModifier * modifier_;

    std::function<void()>
                funcUpdateFromWidgets_,
                funcUpdateWidgets_;

    GroupWidget * group_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_GEOMETRYMODIFIERWIDGET_H
