/** @file frontscene.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 31.01.2015</p>
*/

#ifndef MOSRC_GUI_UTIL_FRONTSCENE_H
#define MOSRC_GUI_UTIL_FRONTSCENE_H

#include <QGraphicsScene>

namespace MO {
class Object;
class Parameter;
namespace GUI {
class AbstractFrontItem;

class FrontScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit FrontScene(QObject *parent = 0);
    ~FrontScene();

signals:

    /** When an item was selected */
    void itemSelected(AbstractFrontItem*);

public slots:

    /** Sets the root object and completely
        (re-)initializes the QGraphicsScene */
    void setRootObject(Object * root);

private slots:

    void onSelectionChanged_();

private:
    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_UTIL_FRONTSCENE_H
