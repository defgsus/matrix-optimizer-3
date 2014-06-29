/** @file scene.h

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_SCENE_H
#define MOSRC_OBJECT_SCENE_H

#include "object.h"


namespace MO {
namespace GL { class Context; }

class Scene : public Object
{
    Q_OBJECT
public:
    explicit Scene(QObject *parent = 0);

    const QString& className() const { static QString s("Scene"); return s; }

    bool isScene() const { return true; }

    // ------------- child objects -------------

    const QList<Camera*> cameras() const { return cameras_; }

    // ------------- open gl -------------------

signals:

public slots:

    /** Tells the Scene to update it's info about the tree */
    void treeChanged();

    // ------------- open gl -------------------

    /** Sets the opengl Context for all objects in the scene. */
    void setGlContext(GL::Context * context);

private:

    // ------------ object collection ----------

    /** Collects all special child objects */
    void findObjects_();

    // ---------- opengl -----------------------

    /** Initializes all opengl childs */
    void initGlChilds_();

    // ---------- opengl -----------------------

    GL::Context * glContext_;

    // ----------- special objects -------------

    QList<Camera*> cameras_;
    QList<ObjectGl*> glObjects_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SCENE_H
