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

    MO_OBJECT_CLONE(Scene)

    const QString& className() const { static QString s(MO_OBJECTCLASSNAME_SCENE); return s; }

    virtual Type type() const { return T_SCENE; }
    bool isScene() const { return true; }

    // ------------- child objects -------------

    const QList<Camera*> cameras() const { return cameras_; }

    // ------------- open gl -------------------

    /** Calculates all transformation of all scene objects.
        @note Scene must be up-to-date with the tree! */
    void calculateSceneTransform(int thread, Double time);

signals:

    /** Scene should be rerendered */
    void renderRequest();

public slots:

    /** Tells the Scene to update it's info about the tree */
    void treeChanged();

    // ------------- parameter -----------------

    void setParameterValue(MO::ParameterFloat *, Double value);

    // ------------- sequences -----------------

    void beginSequenceChange(MO::Sequence *);
    void endSequenceChange();

    // ------------- open gl -------------------

    /** Sets the opengl Context for all objects in the scene. */
    void setGlContext(MO::GL::Context * context);

    /** Render the whole scene on the current context */
    void renderScene(Double time = 0.0);

private:

    // ------------ object collection ----------

    /** Collects all special child objects */
    void findObjects_();

    /** Tell all objects how much threads we got */
    void updateNumberThreads_();

    // ---------- opengl -----------------------

    /** Initializes all opengl childs */
    void initGlChilds_();

    // ---------- opengl -----------------------

    GL::Context * glContext_;

    // ----------- special objects -------------

    QList<Object*> allObjects_;
    QList<Object*> posObjects_;
    QList<Camera*> cameras_;
    QList<ObjectGl*> glObjects_;

    // ---------- properties -------------------

    int numThreads_;

};

} // namespace MO

#endif // MOSRC_OBJECT_SCENE_H
