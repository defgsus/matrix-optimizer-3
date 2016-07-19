/** @file frontview.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#ifndef MO_DISABLE_FRONT

#ifndef MOSRC_GUI_FRONTVIEW_H
#define MOSRC_GUI_FRONTVIEW_H

#include <QWidget>

class QGraphicsView;

namespace MO {
class Object;
class Parameter;
class Modulator;
namespace GUI {

class FrontScene;
class AbstractFrontItem;
class PresetsWidget;

/** Complete view for user interface.
    Contains the QGraphicsView and a PresetsWidget. */
class FrontView : public QWidget
{
    Q_OBJECT
public:
    explicit FrontView(QWidget *parent = 0);

signals:

public slots:

    /** Assigns the scene to display. */
    void setFrontScene(FrontScene *);

    /** Focuses the view on the object */
    void setFocusObject(Object * o);

private slots:

    void onEditModeChange_(bool e);

private:

    void createWidgets_();

    QGraphicsView * gview_;
    FrontScene * gscene_;
    PresetsWidget * presets_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_FRONTVIEW_H

#endif // MO_DISABLE_FRONT
