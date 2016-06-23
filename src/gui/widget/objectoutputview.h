/** @file objectoutputview.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 16.05.2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_OBJECTOUTPUTVIEW_H
#define MOSRC_GUI_WIDGET_OBJECTOUTPUTVIEW_H

#include <QWidget>
#include <QList>
#include <QSize>

#include "object/object_fwd.h"
#include "gl/opengl_fwd.h"
#include "types/float.h"

class QLabel;

namespace MO {
namespace GUI {

/** A display of the contents of objects (or their outputs) */
class ObjectOutputView : public QWidget
{
    Q_OBJECT
public:
    explicit ObjectOutputView(QWidget *parent = 0);
    ~ObjectOutputView();

    Object * object() const { return object_; }

signals:

public slots:

    /** Assigns the object to view, or NULL */
    void setObject(Object *);
    /** Update the view with assigned object */
    void updateObject();
private:

    void createWidgets_();
    void updateLabels_();
    void setLabel_(QPair<QLabel*, const GL::Texture*>&,
                   ValueTextureInterface*, uint channel, Double time);

    Object * object_;
    GL::Manager* manager_;
    QList<QPair<QLabel*, const GL::Texture*>> labels_;
    QSize imgSize_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_OBJECTOUTPUTVIEW_H
