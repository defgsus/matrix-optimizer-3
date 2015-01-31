/** @file frontview.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#ifndef MOSRC_GUI_FRONTVIEW_H
#define MOSRC_GUI_FRONTVIEW_H

#include <QGraphicsView>


namespace MO {
class Object;
class Parameter;
class Modulator;
namespace GUI {

class FrontScene;

class FrontView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit FrontView(QWidget *parent = 0);

signals:

public slots:

    /** Sets the root object and completely
        (re-)initializes the QGraphicsScene */
    void setRootObject(Object * root);

    /** Focuses the view on the object */
    void setFocusObject(Object * o);

private:

    FrontScene * gscene_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_FRONTVIEW_H
