/** @file scene.h

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_SCENE_H
#define MOSRC_OBJECT_SCENE_H

#include <QTimer>

#include "object.h"


namespace MO {
namespace GL { class Context; }

class Scene : public Object
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Scene);

    virtual Type type() const { return T_SCENE; }
    bool isScene() const { return true; }

    // ------------- child objects -------------

    const QList<Camera*> cameras() const { return cameras_; }

    // ------------- open gl -------------------

    /** Calculates all transformation of all scene objects.
        @note Scene must be up-to-date with the tree! */
    void calculateSceneTransform(int thread, Double time);

    // --------------- runtime -----------------

    Double sceneTime() const { return sceneTime_; }

signals:

    /** Scene should be rerendered */
    void renderRequest();

    /** Emitted after settings in an object have changed. */
    void objectChanged(MO::Object *);

    /** Some setting in the Sequence has changed. */
    void sequenceChanged(MO::Sequence *);

public slots:

    /** Tells the Scene to update it's info about the tree */
    void tellTreeChanged();

    // ------------- parameter -----------------

    void setParameterValue(MO::ParameterFloat *, Double value);

    // --------------- tracks ------------------

    // ------------- sequences -----------------

    SequenceFloat * createFloatSequence(MO::Track * track, Double time = 0.0);

    /** Moves the Sequence @seq from Track @p from to different Track @p to.
        The sequence will be removed from the previous track. */
    void moveSequence(MO::Sequence * seq, MO::Track * from, MO::Track * to);

    void beginSequenceChange(MO::Sequence *);
    void endSequenceChange();

    // ---------- objects in general -----------

    void beginObjectChange(MO::Object *);
    void endObjectChange();

    // ------------- open gl -------------------

    /** Sets the opengl Context for all objects in the scene. */
    void setGlContext(MO::GL::Context * context);

    // XXX all hacky right now

    /** Render the whole scene on the current context */
    void renderScene(Double time = 0.0);

    void setSceneTime(Double time) { sceneTime_ = time; emit renderRequest(); }

    void start();
    void stop();

private slots:
    void timerUpdate_();
private:

    // ------------ object collection ----------

    /** Collects all special child objects */
    void findObjects_();

    /** Tell all objects how much threads we got */
    void updateNumberThreads_();

    void updateModulators_();

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

    // ------------ threadstuff ----------------

    Object * changedObject_;
    Sequence * changedSequence_;

    // ------------ runtime --------------------

    QTimer timer_;

    Double sceneTime_;
};

} // namespace MO

#endif // MOSRC_OBJECT_SCENE_H
