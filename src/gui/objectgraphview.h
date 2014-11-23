/** @file objectgraphview.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 23.11.2014</p>
*/

#ifndef MOSRC_GUI_OBJECTGRAPHVIEW_H
#define MOSRC_GUI_OBJECTGRAPHVIEW_H

#include <QGraphicsView>

#include "object/object_fwd.h"

namespace MO {
namespace GUI {


class ObjectGraphView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ObjectGraphView(QWidget *parent = 0);

signals:

public slots:

    /** Sets the root object and completely
        (re-)initializes the QGraphicsScene */
    void setRootObject(Object * root);

private:

    void initScene_();

    QGraphicsScene * gscene_;
    Object * root_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_OBJECTGRAPHVIEW_H
