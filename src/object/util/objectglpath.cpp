/** @file objectglpath.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.01.2015</p>
*/

#include <map>
#include <memory>
#include <functional>

#include "objectglpath.h"
#include "object/scene.h"
#include "object/camera.h"
#include "object/textureoverlay.h"
#include "object/param/modulator.h"
#include "object/util/objecttree.h"
#include "object/util/objectmodulatorgraph.h"
#include "math/transformationbuffer.h"
#include "graph/directedgraph.h"
#include "gl/context.h"
#include "gl/framebufferobject.h"
#include "io/log.h"

namespace MO {
namespace AUDIO { class SpatialSoundSource; }

class ObjectGlPath::Private
{
public:

    /** Wrapper for all objects
        with temporary calculation space */
    struct ObjectBuffer
    {
        ObjectBuffer()
            : object        (0),
              posParent     (0),
              calcMatrix    (0),
              parentMatrix  (0),
              matrix        (0),
              doCalcMatrix  (false)
        { }

        ~ObjectBuffer()
        {
        }

        /// Object associated to this buffer
        Object * object;

        // --- transformations ---
        /// The translation parent (only assigned for translationObjects)
        ObjectBuffer * posParent;
#ifdef MO_DO_DEBUG
        /// Link to the ObjectBuffer that provides the matrix to use for this object.
        /// Might be the object itself or one of it's parents. Only used for debugging.
        ObjectBuffer * matrixParent;
#endif
        /// Calculated matrix
        Mat4 calcMatrix,
        /// Link to parent translation
            *parentMatrix,
        /** Link to the matrix to use for the object.
            Might link to this buffer's calcMatrix or to a parent */
            *matrix;

        bool doCalcMatrix;

        /** Renderlist for cameras */
        QList<ObjectGl*> renderCamList;
    };

    Private(ObjectGlPath * path)
        : path          (path),
          scene         (0),
          context       (0),
          thread        (0),
          isGlInit      (false)
    { }

    ~Private()
    {
        clear();
    }

    void clear();
    void createPath(Scene *);
    /** Always returns a valid buffer */
    ObjectBuffer * getObjectBuffer(Object *);
    /** Returns the buffer, or NULL */
    ObjectBuffer * findObjectBuffer(Object *);

    /** Transforms a list of objects to it's coresponding buffers */
    void createObjectBuffers(const QList<Object*> & input, QList<ObjectBuffer*>& output);

    /** Returns the first parent of @p o (or @p o itself) for which
        the matrix component is set.
        Not all objects have a matrix defined, only audio-relevant objects
        that contain transformations, and the scene.
        This function always returns an object.
        Must be called AFTER preparing the transformationObjects. */
    ObjectBuffer * getParentTransformationObject(ObjectBuffer * o);
    /** Use getParentTransformationObject() to assign the ObjectBuffer::matrix
        to the next parent that actually calculates it's matrix, or to
        the matrix of the scene. */
    void assignMatrix(ObjectBuffer * o);

    /** Prepares the input or output graphic buffers for the objectbuffer.
        Must be called sequentially in dsp order */
    void prepareInputBuffers(ObjectBuffer * o);
    void prepareOutputBuffers(ObjectBuffer * o);

    ObjectGlPath * path;
    Scene * scene;
    GL::Context * context;
    uint thread;
    bool isGlInit;

    // all relevant objects
    std::map<Object *, std::shared_ptr<ObjectBuffer>> objects;

    // separate lists for lookup
    QList<ObjectBuffer*>
        transformationObjects,
        renderObjects;
};

ObjectGlPath::ObjectGlPath()
    : p_    (new Private(this))
{
}

ObjectGlPath::~ObjectGlPath()
{
    delete p_;
}

void ObjectGlPath::createPath(Scene *scene, GL::Context * c, uint thread)
{
    p_->thread = thread;
    p_->context = c;
    p_->createPath(scene);
}

void ObjectGlPath::calcTransformations(Double time)
{
    for (Private::ObjectBuffer * b : p_->transformationObjects)
    {
        if (b->parentMatrix)
        {
            // copy from parent
            b->calcMatrix = *b->parentMatrix;
            // apply transform for one sample
            b->object->calculateTransformation(b->calcMatrix,
                                               time,
                                               p_->thread);
        }
    }
}

#if 0
void ObjectGlPath::calcAudio(SamplePos pos)
{
    // ----------- process audio objects ---------------

    // process audio objects
    for (Private::ObjectBuffer * b : p_->audioObjects)
    {
        auto ao = static_cast<AudioObject*>(b->object);

        // mix input-inbetweens
        // (multiple ins on one audio input)
        for (const Private::InputMixStep & mix : b->audioInputMix)
        {
            mix.buf->writeNullBlock();
            AUDIO::AudioBuffer::mix(mix.inputs, mix.buf);
            mix.buf->nextBlock();
        }

        // process AudioObject
        ao->processAudioBase(p_->conf.bufferSize(), pos, p_->thread);

        // forward buffers
        for (AUDIO::AudioBuffer * buf : b->audioOutputs)
            if (buf)
                buf->nextBlock();
    }

    // clear system audio outputs
    for (AUDIO::AudioBuffer * buf : p_->audioOuts)
        buf->writeNullBlock();

    // mix into system audio outputs
    for (Private::ObjectBuffer * b : p_->audioOutObjects)
        AUDIO::AudioBuffer::mix(b->audioOutputs, p_->audioOuts);


    // ---------- process virtual sound sources --------

    for (Private::ObjectBuffer * b : p_->soundsourceObjects)
    {
        // get transformation-per-soundsource
        b->object->calculateSoundSourceTransformation(
                    b->matrix,
                    b->soundSources,
                    config().bufferSize(),
                    pos, p_->thread);
        // get audio signal per soundsource
        b->object->calculateSoundSourceBuffer(
                    b->soundSources,
                    config().bufferSize(),
                    pos, p_->thread);
        // forward buffers and fill delay-lines
        for (AUDIO::SpatialSoundSource * s : b->soundSources)
        {
            s->signal()->nextBlock();
            s->delay()->writeBlock(s->signal()->readPointer(), config().bufferSize());
        }
    }

    // ------- process virtual microphones -------------

    for (Private::ObjectBuffer * b : p_->microphoneObjects)
    {
        // get transformation-per-microphone
        b->object->calculateMicrophoneTransformation(
                    b->matrix,
                    b->microphones,
                    config().bufferSize(),
                    pos, p_->thread);
        // sample soundsources
        for (AUDIO::SpatialMicrophone * m : b->microphones)
        {
            m->spatialize(b->microphoneInputSoundSources);
            m->signal()->nextBlock();
        }
    }

    // mix into system audio outputs
    for (Private::ObjectBuffer * b : p_->microphoneObjects)
    {
        for (AUDIO::SpatialMicrophone * m : b->microphones)
            if ((int)m->channel() < p_->audioOuts.size())
                p_->audioOuts[m->channel()]->writeAddBlock(
                            m->signal()->readPointer());
    }


    // ----- advance read/write pointer for system audio-outs -----

    for (auto audiobuf : p_->audioOuts)
        audiobuf->nextBlock();
}
#endif

std::ostream& ObjectGlPath::dump(std::ostream & out) const
{
    out << "opengl-graph (" << p_->context << ")\n"
           "transformation objects:";
    for (auto o : p_->transformationObjects)
    {
        out << " " << o->object->name();
        if (o->posParent)
            out << "(" << o->posParent->object->name() << ")";
    }

    out << "\nrender objects:";
    for (auto o : p_->renderObjects)
    {
        out << " " << o->object->name();
#ifdef MO_DO_DEBUG
        out << "(" << o->matrixParent->object->name() << ")";
#endif
    }

//    out << "\ncamera objects:\n";
//    for (Private::ObjectBuffer * o : p_->cameraObjects)
//    {
//    }

    out << std::endl;
    return out;
}


void ObjectGlPath::Private::clear()
{
    scene = 0;
    objects.clear();
    transformationObjects.clear();
    renderObjects.clear();
}

void ObjectGlPath::Private::createPath(Scene * s)
{
    clear();
    scene = s;

    // -------------------- system io ---------------------------



    // ---------------- analyze object tree ---------------------

    // convert to new tree structure
    // XXX There is almost finished work in the newobj branch
    // that uses the ObjectTreeNode for the actual Object tree
    // Here we only use it temporarily because of the
    // nice functionality of TreeNode
    auto tree = get_object_tree(scene);
    std::unique_ptr<ObjectTreeNode> tree_delete(tree);

    // get all modulators in the scene
    QList<Modulator*> all_modulators = scene->getModulators(true);

    // ---- find all objects that need translation calculated ---

    // make a tree of all render-relevant translation objects
    // [the copy function will always copy the root/scene object
    //  so it can serve as the root transformation (identity)]
    auto transformTree = tree->copy(false, [tree](Object * o)
    {
        // XXX need to better check render relevance
        return (tree->find(true, [](Object * o) { return o->isGl(); })
                && o->hasTransformationObjects());
    });

#ifdef MO_GRAPH_DEBUG
    MO_DEBUG("renderTree:");
    transformTree->dumpTree(std::cout);
#endif

    // create object buffers and assign position parents from tree
    transformTree->forEachNode([this](ObjectTreeNode * n)
    {
        ObjectBuffer * b = getObjectBuffer(n->object());
        if (n->parent())
        {
            b->posParent = getObjectBuffer(n->parent()->object());
            b->parentMatrix = &b->posParent->calcMatrix;
        }

        // only render-relevant with transformation, and scene
        b->doCalcMatrix = true;

        transformationObjects.append(b);
    });


    // ---------------- get the render list ----------------

    // cameras
    QList<Camera*> cameras;
    tree->makeLinear<Camera*>(cameras);

    // copy render-dependency to DirectedGraph helper
    DirectedGraph<Object*> rendergraph;

    for (Camera * cam : cameras)
    {
        ObjectBuffer * buf = getObjectBuffer(cam);

        // determine root of camera's render tree
        Object * rroot = cam;
        if (cam->parentObject())
            rroot = cam->parentObject();

        // add dependency of camera to render objects
        auto rtree = get_object_tree(rroot);
        rtree->forEachNode([&rendergraph, buf, cam](ObjectTreeNode * n)
        {
            if (n->object()->isGl() && !n->object()->isCamera())
            {
                rendergraph.addEdge(n->object(), cam);
                // store renderlist per camera
                buf->renderCamList.append(static_cast<ObjectGl*>(n->object()));
            }
        });
    }


#if 0
    for (AudioObject * o : audioObjectList)
    {
        // update sys io objects' channels
        if (auto ao = qobject_cast<AudioInAO*>(o))
            ao->setNumberAudioInputsOutputs(audioIns.size());
        if (auto ao = qobject_cast<AudioOutAO*>(o))
            ao->setNumberAudioInputsOutputs(audioOuts.size());

        // ObjectBuffer for each audio object
        auto b = getObjectBuffer(o);
        audioObjects.append( b );

        // create and link buffers
        prepareAudioInputBuffers(b);
        prepareAudioOutputBuffers(b);

        // tell object
        o->setAudioBuffersBase(thread, conf.bufferSize(), b->audioInputs, b->audioOutputs);
    }


    // ----------- get all soundsource objects ------------------

    QList<Object*> soundsources;
    tree->makeLinear(soundsources, [](const Object*o)
    {
        return o->numberSoundSources();
    });
    // for each object which has soundsources assigned
    for (Object * obj : soundsources)
    {
        auto b = getObjectBuffer(obj);
        soundsourceObjects.append( b );

        // find (parent) translation matrix
        assignMatrix(b);

        // create the soundsources
        for (uint i = 0; i < obj->numberSoundSources(); ++i)
        {
            // direct signal buffer (input to soundsource)
            auto buf = new AUDIO::AudioBuffer(conf.bufferSize());
            auto delay = new AUDIO::AudioDelay(conf.sampleRate() * 2);
            auto src = new AUDIO::SpatialSoundSource(buf, delay);
            b->soundSources.append( src );
        }
    }


    // ----------- get all microphone objects -------------------

    QList<Object*> microphones;
    tree->makeLinear(microphones, [](const Object*o)
    {
        return o->numberMicrophones();
    });
    // for each object which has microphones assigned
    for (Object * obj : microphones)
    {
        auto b = getObjectBuffer(obj);
        microphoneObjects.append( b );

        // find (parent) translation matrix
        assignMatrix(b);

        // create the microphones
        for (uint i = 0; i < obj->numberMicrophones(); ++i)
        {
            // direct signal buffer (output of microphone)
            auto buf = new AUDIO::AudioBuffer(conf.bufferSize());
            auto src = new AUDIO::SpatialMicrophone(buf, conf.sampleRate(), numGlobalMicrophone++);
            b->microphones.append( src );
        }

        // create the list of soundsources that the above microphones
        // should sample from
        // [Right now these are all of them, but i want be able in the future
        //  to just sample sources in the same branch and stuff like that]
        for (Object * sobj : soundsources)
        {
            ObjectBuffer * sbuf = findObjectBuffer(sobj);
            MO_ASSERT(sbuf, "duh?");

            b->microphoneInputSoundSources.append( sbuf->soundSources );
        }
    }
#endif

}







ObjectGlPath::Private::ObjectBuffer * ObjectGlPath::Private::getParentTransformationObject(ObjectBuffer * buf)
{
    if (buf->doCalcMatrix)
        return buf;

    Object * o = buf->object->parentObject();
    buf = findObjectBuffer(o);
    while (!buf || !buf->doCalcMatrix)
    {
        o = o->parentObject();
        buf = findObjectBuffer(o);
    }
    MO_ASSERT(buf, "no transformation parent found");
    return buf;
}

void ObjectGlPath::Private::assignMatrix(ObjectBuffer * b)
{
    ObjectBuffer * buf = getParentTransformationObject(b);
    b->matrix = &buf->calcMatrix;
#ifdef MO_DO_DEBUG
    b->matrixParent = buf;
#endif
}


ObjectGlPath::Private::ObjectBuffer * ObjectGlPath::Private::getObjectBuffer(Object * o)
{
    // return existing?
    auto i = objects.find(o);
    if (i != objects.end())
        return i->second.get();

    // initialize new
    auto buf = new ObjectBuffer();
    buf->object = o;
    // remember
    objects.insert(std::make_pair(o, std::shared_ptr<ObjectBuffer>(buf)));
    return buf;
}

ObjectGlPath::Private::ObjectBuffer * ObjectGlPath::Private::findObjectBuffer(Object * o)
{
    auto i = objects.find(o);
    if (i != objects.end())
        return i->second.get();
    else
        return 0;
}

void ObjectGlPath::Private::createObjectBuffers(const QList<Object *> &input, QList<ObjectBuffer *> &output)
{
    output.clear();
    for (auto o : input)
        output.append( getObjectBuffer(o) );
}




// ---------------------------------- render -------------------------------------------------

bool ObjectGlPath::isGlInitialized() const
{
    return p_->isGlInit;
}

void ObjectGlPath::initGl()
{

}

void ObjectGlPath::render(Double time)
{
    //
}

void ObjectGlPath::releaseGl()
{

}


} // namespace MO
