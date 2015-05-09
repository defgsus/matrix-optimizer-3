/** @file scene.h

    @brief Scene container/controller

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_SCENE_H
#define MOSRC_OBJECT_SCENE_H

#include <QTimer>
#include <QSize>

#include "object.h"
#include "gl/opengl_fwd.h"
#include "gl/lightsettings.h"
#include "audio/audio_fwd.h"

class QReadWriteLock;

namespace MO {
namespace AUDIO { class AudioDevice; }
namespace GUI { class FrontScene; }

class AudioOutThread;
class AudioInThread;
template <typename T> class LocklessQueue;
class ProjectionSystemSettings;


/** Handles tree managment, locking and rendering.
    Audio processing is moved to ObjectDspPath.
    @todo Rendering will be moved to ObjectGlPath some day...
*/
class Scene : public Object
{
    Q_OBJECT

    friend class Object; // for updateTree_()
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

    bool serializeAfterChilds(IO::DataStream&) const Q_DECL_OVERRIDE;
    void deserializeAfterChilds(IO::DataStream&) Q_DECL_OVERRIDE;

    virtual Type type() const { return T_SCENE; }
    bool isScene() const { return true; }

    // ################ PUBLIC INTERFACE ###################

    // ------------- object model --------------

    /** Sets the editor to edit the scene.
        This also assignes the scene to the editor. */
    void setObjectEditor(ObjectEditor * );
    /** Returns the editor that is assigned to this scene. */
    ObjectEditor * editor() const { return editor_; }

    /** A flag to be queried by object when they create runtime resources.
        If this returns true, all resources should be created lazily, so
        that, e.g. a call to ObjectGl::renderGl() always works.
        When false, objects are free to run worker threads that at some point
        come back with the created resource, while rendering is already happening. */
    bool lazyFlag() const { return p_lazyFlag_; }
    void setLazyFlag(bool lazy) { p_lazyFlag_ = lazy; }

    // ------------- child objects -------------

    const QList<Camera*> cameras() const { return cameras_; }

    /** Returns the one clip con, OR NULL */
    ClipController * clipController() const { return clipController_; }

    // --------------- files -------------------

    /** Gets the needed files of ALL objects */
    virtual void getNeededFiles(IO::FileList &files) Q_DECL_OVERRIDE;

    // -------------- info ---------------------

    void setSceneDesc(const QString& desc, bool showOnStart)
        { sceneDesc_ = desc; showSceneDesc_ = showOnStart; }
    const QString& sceneDesc() const { return sceneDesc_; }
    bool showSceneDesc() const { return showSceneDesc_; }

    // ------------- modulators ----------------

    /** Returns a list of all objects that currently serve as modulator */
    QSet<Object*> getAllModulators() const;

    // ------------- ui modulators -------------

    /** Returns (and creates, if necessary) a ModulatorObject as proxy for
        an GUI::AbstractFrontItem. The ModulatorObject will be invisible to
        the user. */
    ModulatorObject * createUiModulator(const QString& uiId);

    /** Returns all ModulatorObjects that belong to the ui item IDs */
    QList<ModulatorObject*> getUiModulatorObjects(const QList<QString>& uiIds) const;
#if 0
    /** Returns all Modulators that belong to the ui item IDs.
        The Modulator will have as source the proxy ModulatorObject and as
        goal the actual goal of the ui controller. */
    QList<Modulator*> getUiModulators(const QList<QString>& uiIds) const;
#endif
    /** Propagates a value from an ui-item to the appropriate ModulatorObjectFloat */
    void setUiValue(const QString& uiId, Double timeStamp, Float value);

    // ------------- ui IO ---------------------

    /** Attaches a FrontScene to the scene.
        The FrontScene will be serialized with the scene.
        Otherwise the FrontScene is not touched. */
    void setFrontScene(GUI::FrontScene * s) { frontScene_ = s; }

    /** Returns the FrontScene xml that has been deserialized.
        If there was no interface saved with the scene, an empty string is returned. */
    QString frontSceneXml() const { return frontSceneXml_; }

    // ------------- open gl -------------------

    // XXX These can be separate per thread!

    /** Returns the currently active framebuffer resolution */
    const QSize & frameBufferSize() const { return fbSize_; }

    uint frameBufferFormat() const { return fbFormat_; }

    /** Returns the resolution of the framebuffer during the next render pass */
    const QSize & requestedFrameBufferSize() const { return fbSizeRequest_; }

    /** Returns the flag if output resolution should be matched by scene fbo resolution */
    bool doMatchOutputResolution() const { return doMatchOutputResolution_; }
    /** Sets the flag if output resolution should be matched by scene fbo resolution.
        This does not do anything else than storing the flag! */
    void setMatchOutputResolution(bool enable) { doMatchOutputResolution_ = enable; }

    QSize outputSize() const { return fbSize_; }

    /** Sets the output framebuffer size.
        This is only a request! The framebuffer will change just
        before the scene is rendered again! */
    void setResolution(const QSize& resolution);

    /** Calculates all transformation of all scene objects.
        @note Scene must be up-to-date with the tree! */
    void calculateSceneTransform(uint thread, Double time);

    /** Sets the options for the debug drawer.
        @p options can be an OR combination of DebugDrawOption enums */
    void setDebugRenderOptions(int options)
        { debugRenderOptions_ = options; render_(); }
    int debugRenderOptions() const { return debugRenderOptions_; }

    // ----------- projection ------------------

    /** Sets the projection settings */
    void setProjectionSettings(const ProjectionSystemSettings &);
    /** The slice camera */
    const ProjectionSystemSettings& projectionSettings() const
        { return * projectionSettings_; }

    /** Sets the index of the projector.
        @note Call setProjectionSettings() before to make
        sure the index is not out of range. */
    void setProjectorIndex(uint index);
    uint projectorIndex() const { return projectorIndex_; }

    // ----------- audio info ------------------

    AudioObjectConnections * audioConnections() { return audioCon_; }
    const AudioObjectConnections * audioConnections() const { return audioCon_; }

    // --------------- runtime -----------------

    Double sceneTime() const { return sceneTime_; }

    void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    /** Call this to update the samplerate for the whole scene.
        To be done before creating the audio dsp path */
    void setSceneSampleRate(uint samplerate);

    /** Executes all script objects */
    void runScripts();

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
    void notifyParameterVisibility(Parameter * p);

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

    /** Send when a parameter changed it's visibility */
    void parameterVisibilityChanged(MO::Parameter*);

    void CameraFboChanged(Camera *);
    void sceneFboChanged();

public slots:

    // --------------- tree --------------------

    /** @defgroup scene_lock threadsafe Scene functions
     *  These functions are all locked, so it's safe to call them anywhere.
     *  @{
     */

    void addObject(Object * parent, Object * newChild, int insert_index = -1);
    void addObjects(Object * parent, const QList<Object*>& newChilds, int insert_index = -1);
    void deleteObject(Object * object);
    void deleteObjects(const QList<Object*>& objects);
    bool setObjectIndex(Object * object, int newIndex);
    void moveObject(Object * object, Object * newParent, int newIndex);

    /** @} */

    // ------------- runtime -------------------

    /** Call before deleting the scene.
        OpenGL resources will be released a short while later from their particular thead.
        */
    void kill();

    /** Sets the scope for enabling all the objects */
    void setSceneActivityScope(ActivityScope scope) { setCurrentActivityScope(scope); render_(); }

    void setSceneTime(Double time, bool send_signal = true);
    void setSceneTime(SamplePos pos, bool send_signal = true);

    /** Send keydown into system.
        XXX a hack basically - and not used yet */
    void keyDown(int key);
    void keyUp(int key);

    // ------------- open gl -------------------

    bool isShutDown() const { return isShutDown_; }

    /** Returns currently set opengl context of all objects in scene. */
    GL::Context * glContext() const { return glContext_; }

    /** Sets the opengl Context for all objects in the scene. */
    void setGlContext(uint thread, MO::GL::Context * context);

    /** Requests rendering of the scene. */
    void render() { render_(); }

    /** Sets the index of the camera to control will setFreeCameraMatrix(),
        -1 for no free camera. */
    void setFreeCameraIndex(int index);
    int freeCameraIndex() const { return freeCameraIndex_; }

    /** Sets the camera matrix in free-camera-mode */
    void setFreeCameraMatrix(const MO::Mat4& mat);

    /** Returns the lighting settings for the scene.
        This may only be valid during rendering in objects! */
    const GL::LightSettings& lightSettings(uint thread) const { return lightSettings_[thread]; }

    /** Returns the framebuffer of the final master frame, or NULL */
    GL::FrameBufferObject * fboMaster(uint thread) const;

    /** Returns the framebuffer of the camera frame, or NULL */
    GL::FrameBufferObject * fboCamera(uint thread, uint camera_index) const;

    /** Render the whole scene on the current context.
        If @p fbo is set, the scene will be rendered into the framebuffer object. */
    void renderScene(Double time, uint thread, bool paintToScreen = true);//, GL::FrameBufferObject * fbo = 0);

private slots:

private:

    // ------------ object collection ----------

    /** Calls onObjectsDeleted() for every object in @p toTell. */
    void tellObjectsAboutToDelete_(const QList<Object*> & toTell, const QList<Object *> & deleted);

    /** Does everything to update the tree */
    void updateTree_();

    /** Collects all special child objects */
    void findObjects_();

    void updateChildrenChanged_();
    /** Tells all objects how much threads we got */
    void updateNumberThreads_();
    /** Tells the objects the samplerate */
    void updateSampleRate_();

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

    // ------------- files ---------------------

    void getNeededFiles_(Object *, IO::FileList&);

    // ----------- runtime ---------------------

    void lockRead_();
    void lockWrite_();
    void unlock_();

    /** unlocked version */
    void calculateSceneTransform_(uint thread, Double time);

    // ---------- opengl -----------------------

    /* Initializes all opengl childs */
    //void initGlChilds_();

    /** Fulfills a resize/format request */
    void resizeFbo_(uint thread);

    /** Fills the LightSettings class with info from the ready transformed tree */
    void updateLightSettings_(uint thread, Double time);

    /** Emits renderRequest if scene is not already running. */
    void render_();

    // -------------- model --------------------

    ObjectEditor * editor_;
    GUI::FrontScene * frontScene_;
    QString frontSceneXml_;

    // ------------------ desc -----------------

    QString sceneDesc_;
    bool showSceneDesc_;

    // ---------- opengl -----------------------

    GL::Context * glContext_;

    std::vector<bool> releaseAllGlRequested_;

    QSize fbSize_;
    uint fbFormat_;
    QSize fbSizeRequest_;
    uint fbFormatRequest_;

    bool doMatchOutputResolution_,
         isShutDown_;

    std::vector<GL::FrameBufferObject *> fboFinal_;
    std::vector<GL::ScreenQuad *> screenQuad_;
    std::vector<GL::LightSettings> lightSettings_;

    std::vector<GL::SceneDebugRenderer*> debugRenderer_;
    int debugRenderOptions_;

    int freeCameraIndex_;
    Mat4 freeCameraMatrix_;

    ProjectionSystemSettings * projectionSettings_;
    uint projectorIndex_;

    // ----------- special objects -------------

    ClipController * clipController_;
    QList<Object*> allObjects_;
    QList<Object*> posObjects_;
    //QList<Object*> posObjectsAudio_;
    QList<Camera*> cameras_;
    QList<ObjectGl*> glObjects_, frameDrawers_;
    QList<ShaderObject*> shaderObjects_;
    //QList<Object*> audioObjects_;
    //QList<Object*> microphoneObjects_;
    QList<LightSource*> lightSources_;
    //QList<AudioUnit*> topLevelAudioUnits_;
    QList<Object*> deletedObjects_;
    QMap<QString, ModulatorObjectFloat*> uiModsFloat_;

    // ---------- properties -------------------

    uint sceneNumberThreads_;
    uint sceneSampleRate_;

    // ------------ threadstuff ----------------

    Object * changedObject_, * changedTimelineObject_,
            * changedTreeObject_;
    Sequence * changedSequence_;

    // ----------- audio ----------------------

    AudioObjectConnections * audioCon_;

    // ------------ runtime --------------------

    QReadWriteLock * readWriteLock_;

    bool isPlayback_, p_lazyFlag_;

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
