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
#include "gl/opengl_fwd.h"
#include "gl/lightsettings.h"
#include "audio/audio_fwd.h"

class QReadWriteLock;

namespace MO {
namespace AUDIO { class AudioDevice; }

class ObjectTreeModel;
class AudioOutThread;
class AudioInThread;
template <typename T> class LocklessQueue;

/** Handles tree managment, locking, rendering and audio processing */
class Scene : public Object
{
    Q_OBJECT

    // for updateTree_()
    friend class Object;
    friend class ScopedSceneLockRead;
    friend class ScopedSceneLockWrite;
    friend class AudioOutThread;
    friend class AudioInThread;
public:

    /** Enables drawing of debug objects */
    enum DebugRenderOption
    {
        DD_AUDIO_SOURCES    = 1,
        DD_MICROPHONES      = 1<<1,
        DD_CAMERAS          = 1<<2,
        DD_LIGHT_SOURCES    = 1<<3
    };


    MO_OBJECT_CONSTRUCTOR(Scene);
    ~Scene();

    virtual Type type() const { return T_SCENE; }
    bool isScene() const { return true; }

    // ################ PUBLIC INTERFACE ###################

    // ------------- object model --------------

    /** Sets the model to edit the scene. */
    void setObjectModel(ObjectTreeModel *);
    /** Returns the model that is assigned to this scene. */
    ObjectTreeModel * model() const { return model_; }

    // ------------- child objects -------------

    const QList<Camera*> cameras() const { return cameras_; }

    // ------------- open gl -------------------

    // XXX These can be separate per thread!
    uint frameBufferWidth() const { return fbWidth_; }
    uint frameBufferHeight() const { return fbHeight_; }
    uint frameBufferFormat() const { return fbFormat_; }
    uint frameBufferCubeMapWidth() const { return fbCmWidth_; }
    uint frameBufferCubeMapHeight() const { return fbCmHeight_; }

    /** Calculates all transformation of all scene objects.
        @note Scene must be up-to-date with the tree! */
    void calculateSceneTransform(uint thread, uint sample, Double time);

    /** Sets the options for the debug drawer.
        @p options can be an OR combination of DebugDrawOption enums */
    void setDebugRenderOptions(int options)
        { debugRenderOptions_ = options; render_(); }
    int debugRenderOptions() const { return debugRenderOptions_; }

    // ----------- audio info ------------------

    uint numberChannelsIn() const { return numInputChannels_; }
    uint numberChannelsOut() const { return numOutputChannels_; }
    const F32 * outputLevels() const { return &outputEnvelopes_[0]; }

    uint numMicrophones() const { return numMicrophones_; }
    uint numAudioSources() const { return allAudioSources_.size(); }

    const QList<AUDIO::AudioSource*>& allAudioSources() const { return allAudioSources_; }

    // --------------- runtime -----------------

    Double sceneTime() const { return sceneTime_; }

    bool isPlayback() const { return isPlayback_; }

    bool isAudioInitialized() const;

    const AUDIO::AudioDevice * audioDevice() const { return audioDevice_; }

    void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    void setBufferSize(uint bufferSize, uint thread) Q_DECL_OVERRIDE;

    // --------- locking and updates -----------

    void beginSequenceChange(MO::Sequence *);
    void endSequenceChange();

    void beginTimelineChange(MO::Object *);
    void endTimelineChange();

    /** Actually for changing parameters */
    void beginObjectChange(MO::Object *);
    void endObjectChange();

    // --------- parameters --------------------

    /** Used by Parameter to emit a parameterVisibilityChanged() */
    void notifyParameterVisibility(Parameter * p)
        { emit parameterVisibilityChanged(p); }

signals:

    /** Scene should be re-rendered */
    void renderRequest();

    void playbackStarted();
    void playbackStopped();

    /** Emitted when the number of channels is set/changed */
    void numberChannelsChanged(uint numIn, uint numOut);

    /** Emitted when the number of (currently) microphones changed */
    void numberOutputEnvelopesChanged(uint num);

    /** This is send regularily during playback, representing the microphone levels */
    void outputEnvelopeChanged(const F32 * levels);

    /** Emitted whenever the scene time changed */
    void sceneTimeChanged(Double);

    /** openGL resources have been released for the given thread. */
    void glReleased(uint thread);

    /** A parameter has been changed (with Scene::setParameter..) */
    void parameterChanged(MO::Parameter*);

    /** Send when a parameter changed it's visibility */
    void parameterVisibilityChanged(MO::Parameter*);

    /** Emitted after settings in an object have changed. */
    void objectChanged(MO::Object *);

    /** Some setting in the Sequence has changed. */
    void sequenceChanged(MO::Sequence *);

    /** Emitted when the given object was added to the scene. */
    void objectAdded(MO::Object *);

    /** Emitted when the object was deleted.
        The object pointer will not point to a valid object anymore. */
    void objectDeleted(const MO::Object *);

    /** Emitted after swapChildren() */
    void childrenSwapped(MO::Object *, int from, int to);

public slots:

    // --------------- tree --------------------

    // these are all locked

    void addObject(Object * parent, Object * newChild, int insert_index = -1);
    void deleteObject(Object * object);
    void swapChildren(Object * parent, int from, int to);

    // ------------- parameter -----------------

    void setParameterValue(MO::ParameterInt *, Int value);
    void setParameterValue(MO::ParameterFloat *, Double value);
    void setParameterValue(MO::ParameterSelect *, int value);
    void setParameterValue(MO::ParameterFilename *, const QString& value);
    void setParameterValue(MO::ParameterText *, const QString& value);
    void addModulator(MO::Parameter *, const QString& idName);
    void removeModulator(MO::Parameter *, const QString& idName);
    void removeAllModulators(MO::Parameter *);

    // --------------- tracks ------------------

    // ------------- sequences -----------------

    //SequenceFloat * createFloatSequence(MO::Track * track, Double time = 0.0);

    /* Moves the Sequence @seq from Track @p from to different Track @p to.
        The sequence will be removed from the previous track. */
    //void moveSequence(MO::Sequence * seq, MO::Track * from, MO::Track * to);

    // ------------- audiounits ----------------

    /** Updates input/output channel sizes of all audiounits.
        Locked version */
    void updateAudioUnitChannels();

    // ------------- runtime -------------------

    /** Call before deleting the scene.
        OpenGL resources will be released a short while later from their particular thead.
        */
    void kill();

    /** Sets the scope for enabling all the objects */
    void setSceneActivityScope(ActivityScope scope) { setCurrentActivityScope(scope); render_(); }

    void setSceneTime(Double time, bool send_signal = true);
    void setSceneTime(SamplePos pos, bool send_signal = true);

    // -------------- audio --------------------

    /** Performs one audio block calculation of the whole scene. */
    void calculateAudioBlock(SamplePos samplePos, uint thread);

    /** Returns a pointer to the audio block previously calculated by calculateAudioBlock().
        The format in the returned pointer is [microphone][buffersize]. */
    const F32* getMicrophonesOutput(uint thread) const { return &sceneAudioOutput_[thread][0]; }

    /** Returns the final audio output previously calculated by calculateAudioBlock() in @p buffer.
        The format of buffer is [buffersize][channel]. */
    void getAudioOutput(uint numChannels, uint thread, F32 * buffer) const;

    // ------------- open gl -------------------

    /** Sets the opengl Context for all objects in the scene. */
    void setGlContext(uint thread, MO::GL::Context * context);

    /** Requests rendering of the scene. */
    void render() { render_(); }

    /** Sets the index of the camera to control will setFreeCameraMatrix(),
        -1 for no free camera. */
    void setFreeCameraIndex(int index) { freeCameraIndex_ = index; render_(); }
    int freeCameraIndex() const { return freeCameraIndex_; }

    /** Sets the camera matrix in free-camera-mode,
        used when scene is rendered next time. */
    void setFreeCameraMatrix(const MO::Mat4& mat) { freeCameraMatrix_ = mat; render_(); }

    /** Returns the lighting settings for the scene.
        This may only be valid during rendering in objects! */
    const GL::LightSettings& lightSettings(uint thread) const { return lightSettings_[thread]; }

    /** Returns the framebuffer of the final master frame, or NULL */
    GL::FrameBufferObject * fboMaster(uint thread) const;

    /** Returns the framebuffer of the camera frame, or NULL */
    GL::FrameBufferObject * fboCamera(uint thread, uint camera_index) const;

    /** Render the whole scene on the current context */
    void renderScene(uint thread);

    /** Start realtime playback */
    void start();
    /** Stop realtime playback */
    void stop();

    /** Closes the audio device stream */
    void closeAudio();

private slots:

private:

    // ------------ object collection ----------

    /** Calls onObjectsDeleted() for every object in @p toTell. */
    void tellObjectsAboutToDelete_(const QList<Object*> & toTell, const QList<Object *> & deleted);

    /** Does everything to update the tree */
    void updateTree_();

    /** Request from object @p o to call it's createOutputs() method. */
    void callCreateOutputs_(Object * o);

    /** Request from object @p o to call it's createMicrophones() method. */
    void callCreateMicrophones_(Object * o);

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
    /** Creates EnvelopeFollower for one thread only!
        AudioDevice must be initialized */
    void allocateAudioOutputEnvelopes_(uint thread);
    void updateModulators_();

    /** Tell everyone the number of light sources */
    void updateNumberLights_();

    /** Creates the framebuffer object */
    void createSceneGl_(uint thread);
    void releaseSceneGl_(uint thread);

    /** Safely destroys all deletedObjects_.
        if @p releaseGl is true, their Object::releaseGl() functions
        will be called for each thread if needed. */
    void destroyDeletedObjects_(bool releaseGl);

    // ----------- runtime ---------------------

    void lockRead_();
    void lockWrite_();
    void unlock_();

    /** unlocked version */
    void calculateSceneTransform_(uint thread, uint sample, Double time);
    void calculateAudioSceneTransform_(uint thread, uint sample, Double time);

    // ------------ audio ----------------------

    void initAudioDevice_();
    void audioCallback_(const F32 *, F32 *);

    /** Rearranges the input from audio api to internal buffer */
    void prepareAudioInputBuffer_(uint thread);
    /** Sets the number of input channels for all (top-level) audioUnits
        and adjusts their children accordingly. */
    void updateAudioUnitChannels_();

    /** Transforms the channel-layout and fills internal buffer */
    void transformAudioInput_(const F32 * in, uint thread);

    /** Processes all top-level AudioUnits + their childs */
    void processAudioInput_(uint thread);

    /** Passes the internal audio output buffer to the envelope followers */
    void updateOutputEnvelopes_(uint thread);

    // ---------- opengl -----------------------

    /* Initializes all opengl childs */
    //void initGlChilds_();

    /** Fills the LightSettings class with info from the ready transformed tree */
    void updateLightSettings_(uint thread, Double time);

    /** Emits renderRequest if scene is not already running. */
    void render_();

    // -------------- model --------------------

    ObjectTreeModel * model_;

    // ---------- opengl -----------------------

    GL::Context * glContext_;

    std::vector<bool> releaseAllGlRequested_;

    uint fbWidth_, fbHeight_, fbFormat_,
        fbCmWidth_, fbCmHeight_;

    std::vector<GL::FrameBufferObject *> fboFinal_;
    std::vector<GL::ScreenQuad *> screenQuad_;
    std::vector<GL::LightSettings> lightSettings_;

    std::vector<GL::SceneDebugRenderer*> debugRenderer_;
    int debugRenderOptions_;

    int freeCameraIndex_;
    Mat4 freeCameraMatrix_, freeCameraMatrixGfx_;
    std::vector<Mat4> freeCameraMatrixAudio_;

    // ----------- special objects -------------

    QList<Object*> allObjects_;
    QList<Object*> posObjects_;
    QList<Object*> posObjectsAudio_;
    QList<Camera*> cameras_;
    QList<ObjectGl*> glObjects_;
    QList<Object*> audioObjects_;
    QList<Object*> microphoneObjects_;
    QList<LightSource*> lightSources_;
    QList<AudioUnit*> topLevelAudioUnits_;
    QList<Object*> deletedObjects_;

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

    AudioInThread * audioInThread_;
    AudioOutThread * audioOutThread_;
    LocklessQueue<const F32*> * audioInQueue_;
    LocklessQueue<F32*> * audioOutQueue_;

    uint numMicrophones_;

    QList<AUDIO::AudioSource*> allAudioSources_;

    /** [thread] [microphones][bufferSize] */
    std::vector<std::vector<F32>> sceneAudioOutput_;
    /** [channels][bufferSize] */
    std::vector<F32> sceneAudioInput_,
    /** [numInputBuffers][bufferSize][channels] */
        apiAudioInputBuffer_;

    bool isFirstAudioCallback_;
    uint numInputChannels_,
         numOutputChannels_,
    /** How much buffer-blocks to store of input */
         numInputBuffers_,
         curInputBuffer_;

    std::vector<AUDIO::EnvelopeFollower*> outputEnvelopeFollower_;
    std::vector<F32> outputEnvelopes_;

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
