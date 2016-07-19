/** @file objectgraphview.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#ifndef MOSRC_GUI_OBJECTGRAPHVIEW_H
#define MOSRC_GUI_OBJECTGRAPHVIEW_H

#include <QWidget>

#include "object/Object_fwd.h"

class QGraphicsView;

namespace MO {
class ActionList;
namespace GUI {

class ObjectGraphScene;
class SceneSettings;

class ObjectGraphView : public QWidget
{
    Q_OBJECT
public:
    explicit ObjectGraphView(QWidget *parent = 0);

signals:

    /** An object has been clicked */
    void objectSelected(MO::Object *);

    /** Actions for edit-mainmenu have been populated. */
    void editActionsChanged(const ActionList&);

public slots:

    /** Call this before setting the root object */
    void setGuiSettings(SceneSettings*);

    /** Sets the root object and completely
        (re-)initializes the QGraphicsScene */
    void setRootObject(Object * root);

    /** Focuses the view on the object */
    void setFocusObject(Object * o);

    /** Zoom in (true) or out (false) */
    void zoom(bool in);

private slots:

    void onShiftView_(const QPointF&);

protected:

    void keyPressEvent(QKeyEvent*);


private:

    void createWidgets_();

    QGraphicsView * gview_;
    ObjectGraphScene * gscene_;
    Object * root_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_OBJECTGRAPHVIEW_H
