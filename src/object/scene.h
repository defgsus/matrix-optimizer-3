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

class QReadWriteLock;

namespace MO {
namespace AUDIO { class AudioDevice; }
namespace GL { class Context; }

class ObjectTreeModel;

class Scene : public Object
{
    Q_OBJECT

    // for updateTree_()
    friend class Object;
    friend class ScopedSceneLockRead;
    friend class ScopedSceneLockWrite;

public:
    MO_OBJECT_CONSTRUCTOR(Scene);
    ~Scene();

    virtual Type type() const { return T_SCENE; }
    bool isScene() const { return true; }

    // ------------- object model --------------

    /** Sets the model to edit the scene. */
    void setObjectModel(ObjectTreeModel *);
    /** Returns the model that is assigned to this scene. */
    ObjectTreeModel * model() const { return model_; }

    // ------------- child objects -------------

    const QList<Camera*> cameras() const { return cameras_; }
    const QList<Microphone*> microphones() const { return microphones_; }

    // ------------- open gl -------------------

    /** Calculates all transformation of all scene objects.
        @note Scene must be up-to-date with the tree! */
    void calculateSceneTransform(uint thread, uint sample, Double time);

    // --------------- runtime -----------------

    Double sceneTime() const { return sceneTime_; }

    bool isPlayback() const { return isPlayback_; }

    bool isAudioInitialized() const;

    const AUDIO::AudioDevice * audioDevice() const { return audioDevice_; }

    // --------- locking and updates -----------

    void beginSequenceChange(MO::Sequence *);
    void endSequenceChange();

    void beginTimelineChange(MO::Object *);
    void endTimelineChange();

    /** Actually for changing parameters */
    void beginObjectChange(MO::Object *);
    void endObjectChange();

signals:

    /** Scene should be rerendered */
    void renderRequest();

    void playbackStarted();
    void playbackStopped();

    /** Emitted whenever the scene time changed */
    void sceneTimeChanged(Double);

    /** A parameter has been changed (with Scene::setParameter..) */
    void parameterChanged(MO::Parameter*);

    /** Emitted after settings in an object have changed. */
    void objectChanged(MO::Object *);

    /** Some setting in the Sequence has changed. */
    void sequenceChanged(MO::Sequence *);

    /** Emitted when the given object was added to the scene. */
    void objectAdded(MO::Object *);

    /** Emitted when the object was deleted.
        The object pointer will not point to a valid object anymore. */
    void objectDeleted(MO::Object *);

    /** Emitted after swapChildren() */
    void childrenSwapped(MO::Object *, int from, int to);

public slots:

    // --------------- tree --------------------

    // these are all locked

    void addObject(Object * parent, Object * newChild, int insert_index = -1);
    void deleteObject(Object * object);
    void swapChildren(Object * parent, int from, int to);

    // ------------- parameter -----------------

    void setParameterValue(MO::ParameterFloat *, Double value);
    void setParameterValue(MO::ParameterSelect *, int value);
    void addModulator(MO::Parameter *, const QString& idName);
    void removeModulator(MO::Parameter *, const QString& idName);
    void removeAllModulators(MO::Parameter *);

    // --------------- tracks ------------------

    // ------------- sequences -----------------

    //SequenceFloat * createFloatSequence(MO::Track * track, Double time = 0.0);

    /* Moves the Sequence @seq from Track @p from to different Track @p to.
        The sequence will be removed from the previous track. */
    //void moveSequence(MO::Sequence * seq, MO::Track * from, MO::Track * to);

    // ------------- runtime -------------------

    /** Sets the scope for enabling all the objects */
    void setSceneActivityScope(ActivityScope scope) { setCurrentActivityScope(scope); render_(); }

    void setSceneTime(Double time, bool send_signal = true);
    void setSceneTime(SamplePos pos, bool send_signal = true);

    // -------------- audio --------------------

    /** Performs one audio block calculation of the whole scene. */
    void calculateAudioBlock(SamplePos samplePos, uint thread);

    /** Returns a pointer to the audio block previously calculated by calculateAudioBlock().
        The format in the returned pointer is [microphone][buffersize]. */
    const F32* getMicrophonesOutput(uint thread) const { return &audioOutput_[thread][0]; }

    /** Returns the final audio output previously calculated by calculateAudioBlock() in @p buffer.
        The format of buffer is [buffersize][channel]. */
    void getAudioOutput(uint numChannels, uint thread, F32 * buffer) const;

    // ------------- open gl -------------------

    /** Sets the opengl Context for all objects in the scene. */
    void setGlContext(MO::GL::Context * context);

    // XXX all hacky right now

    /** Render the whole scene on the current context */
    void renderScene(Double time = 0.0, uint thread = 0);

    /** Start realtime playback */
    void start();
    /** Stop realtime playback */
    void stop();

    /** Closes the audio device stream */
    void closeAudio();

private slots:
    void timerUpdate_();
private:

    // ------------ object collection ----------

    /** Does everything to update the tree */
    void updateTree_();

    /** Collects all special child objects */
    void findObjects_();

    void updateChildrenChanged_();
    /** Tells all objects how much threads we got */
    void updateNumberThreads_();
    /** Tells the objects the buffersize for each thread */
    void updateBufferSize_();
    /** Tells the objects the samplerate */
    void updateSampleRate_();
    /** Tells the objects' audio sources the delay size for each thread */
    void updateDelaySize_();

    /** Initializes the audio buffers needed to render the tree */
    void updateAudioBuffers_();

    void updateModulators_();

    // ----------- runtime ---------------------

    void lockRead_();
    void lockWrite_();
    void unlock_();

    /** unlocked version */
    void calculateSceneTransform_(uint thread, uint sample, Double time);

    void initAudioDevice_();
    void audioCallback_(const F32 *, F32 *);

    // ---------- opengl -----------------------

    /* Initializes all opengl childs */
    //void initGlChilds_();

    /** Emits renderRequest if scene is not already running. */
    void render_();

    // -------------- model --------------------

    ObjectTreeModel * model_;

    // ---------- opengl -----------------------

    GL::Context * glContext_;

    // ----------- special objects -------------

    QList<Object*> allObjects_;
    QList<Object*> posObjects_;
    QList<Camera*> cameras_;
    QList<ObjectGl*> glObjects_;
    QList<Object*> audioObjects_;
    QList<Microphone*> microphones_;

    // ---------- properties -------------------

    uint sceneNumberThreads_;
    std::vector<uint>
        sceneBufferSize_,
        sceneDelaySize_;
    uint sceneSampleRate_;

    // ------------ threadstuff ----------------

    Object * changedObject_, * changedTimelineObject_,
            * changedTreeObject_;
    Sequence * changedSequence_;

    // ----------- audio ----------------------

    AUDIO::AudioDevice * audioDevice_;

    /** [thread] [microphones][bufferSize] */
    std::vector<std::vector<F32>> audioOutput_;

    // ------------ runtime --------------------

    QReadWriteLock * readWriteLock_;
    //QTimer timer_;

    bool isPlayback_;

    Double sceneTime_;
    SamplePos samplePos_;
};


/* XXX right now only used for adding floattracks to a parameter */
class ScopedObjectChange
{
    Scene * scene_;
public:
    ScopedObjectChange(Scene * scene, Object * object)
        :   scene_(scene)
    {
        scene_->beginObjectChange(object);
    }

    ~ScopedObjectChange() { scene_->endObjectChange(); }
};


class ScopedSequenceChange
{
    Scene * scene_;
public:
    ScopedSequenceChange(Scene * scene, Sequence * sequence)
        :   scene_(scene)
    {
        scene_->beginSequenceChange(sequence);
    }

    ~ScopedSequenceChange() { scene_->endSequenceChange(); }
};






} // namespace MO

#endif // MOSRC_OBJECT_SCENE_H
