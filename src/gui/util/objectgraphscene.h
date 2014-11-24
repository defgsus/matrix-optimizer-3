/** @file objectgraphscene.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.11.2014</p>
*/

#ifndef MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H
#define MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H

#include <QGraphicsScene>

namespace MO {
class Object;
namespace GUI {

class AbstractObjectItem;

class ObjectGraphScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit ObjectGraphScene(QObject *parent = 0);
    ~ObjectGraphScene();

    // -------------- getter -------------------

    /** Return the item for the object, or NULL */
    AbstractObjectItem * itemForObject(Object * o) const;

    /** Return the first visible item or one of it's parents for the object, or NULL */
    AbstractObjectItem * visibleItemForObject(Object * o) const;

signals:

public slots:

    /** Sets the root object and completely
        (re-)initializes the QGraphicsScene */
    void setRootObject(Object * root);

private slots:

    void onChanged_();

private:

    class Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_UTIL_OBJECTGRAPHSCENE_H
