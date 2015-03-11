/** @file objectdsppath.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 01.12.2014</p>
*/

/** @page dsp_path


  */

#include <map>
#include <memory>
#include <functional>

#include "objectdsppath.h"
#include "object/scene.h"
#include "object/microphone.h"
#include "object/audioobject.h"
#include "object/audio/audiooutao.h"
#include "object/audio/audioinao.h"
#include "object/param/modulator.h"
#include "object/util/objecttree.h"
#include "object/util/audioobjectconnections.h"
#include "object/util/objectmodulatorgraph.h"
#include "audio/configuration.h"
#include "audio/tool/audiobuffer.h"
#include "audio/tool/delay.h"
#include "audio/spatial/spatialsoundsource.h"
#include "audio/spatial/spatialmicrophone.h"
#include "math/transformationbuffer.h"
#include "graph/directedgraph.h"
#ifndef MO_DISABLE_SERVER
#   include "engine/serverengine.h"
#   include "network/udpaudioconnection.h"
#endif
#ifndef MO_DISABLE_CLIENT
#   include "network/udpaudioconnection.h"
#endif
#include "io/log.h"

namespace MO {
namespace AUDIO { class SpatialSoundSource; }

class ObjectDspPath::Private
{
public:

    struct InputMixStep
    {
        explicit InputMixStep(AUDIO::AudioBuffer * buf)
            : buf(buf), buf_(buf)
        { }

        AUDIO::AudioBuffer * buf;
        QList<AUDIO::AudioBuffer*> inputs;
    private:
        std::shared_ptr<AUDIO::AudioBuffer> buf_;
    };

    /** Wrapper for all objects
        with temporary calculation space */
    struct ObjectBuffer
    {
        ObjectBuffer()
            : object        (0),
              posParent     (0),
              calcMatrix    (0),
              parentMatrix  (0),
              matrix        (0)
#ifndef MO_DISABLE_CLIENT
              , udpInput    (0)
#endif
#ifndef MO_DISABLE_SERVER
              , udpOutput   (0)
#endif
        { }

        ~ObjectBuffer()
        {
            for (auto b : audioOutputs)
                delete b;

            for (auto s : soundSources)
            {
                delete s->signal();
                delete s->delay();
                delete s;
            }

            for (auto s : microphones)
            {
                delete s->signal();
                delete s;
            }
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
        TransformationBuffer calcMatrix,
        /// Link to parent translation
            *parentMatrix,
        /** Link to the matrix to use for the object.
            Might link to this buffer's calcMatrix or to a parent */
            *matrix;

        // --- spatialization ---
        QList<AUDIO::SpatialSoundSource*>
        /// Soundsources per object (memory managed)
            soundSources,
        /// All sound sources that the microphones must record
            microphoneInputSoundSources;
        QList<AUDIO::SpatialMicrophone*>
        /// Microphones per object (memory managed)
            microphones;

        // --- audio objects ---
        QList<AUDIO::AudioBuffer*>
        /// Audio output buffers (memory managed)
            audioOutputs,
        /// Links connected audio output buffers
            audioInputs;
        QList<InputMixStep>
        /// input buffer mix space (memory managed)
            audioInputMix;

#ifndef MO_DISABLE_CLIENT
        /// Connection to server audio signals, or NULL (serves as flag as well)
        UdpAudioConnection * udpInput;
        /// List of output buffers that need to be received
        QList<AUDIO::AudioBuffer*> udpInputBuffers;
#endif
#ifndef MO_DISABLE_SERVER
        /// audio stream to clients, or NULL (serves as flag as well)
        UdpAudioConnection * udpOutput;
        /// List of buffers that need to be send
        QList<AUDIO::AudioBuffer*> udpOutputBuffers;
#endif
    };

    Private(ObjectDspPath * path)
        : path          (path),
          scene         (0)
#ifndef MO_DISABLE_SERVER
          , udpOutput   (0)
#endif
#ifndef MO_DISABLE_CLIENT
          , udpInput    (0)
#endif
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

    /** Adds the audioToFloatConverter type modulator to the audioToModulator set */
    void addAudioToModulator(Modulator * m);
    /** Returns true if output channel of ao links to a Modulator */
    bool hasAudioToModulator(AudioObject * ao, uint channel);
    /** Returns true if any channel links to a modulator */
    bool hasAudioToModulator(AudioObject * ao);

    //void assignMicrophoneInputSoundSources(ObjectBuffer * o);

    /** Prepares the audio input buffers for the objectbuffer.
        Must be called sequentially in dsp order */
    void prepareAudioInputBuffers(ObjectBuffer * o);
    void prepareAudioOutputBuffers(ObjectBuffer * o);
    /** Separate setup of udp buffers to create them from within the audio thread */
    void prepareUdpOutputs(ObjectBuffer * o);

    void prepareSoundSourceBuffer(ObjectBuffer * o);

#ifndef MO_DISABLE_SERVER
    /** Creates and/or returns the single associated to-client output stream */
    UdpAudioConnection * getUdpOutput();
#endif
#ifndef MO_DISABLE_CLIENT
    /** Creates and/or returns the single associated from-server input stream */
    UdpAudioConnection * getUdpInput();
#endif

    ObjectDspPath * path;
    Scene * scene;
    AUDIO::Configuration conf;
    uint thread;

    // all relevant objects
    std::map<Object *, std::shared_ptr<ObjectBuffer>> objects;
    // all audio outputs mapping to modulator inputs
    QMap<AudioObject*, QSet<uint>> audioToModulator;

    QList<ObjectBuffer*>
        transformationObjects,
        soundsourceObjects,
        microphoneObjects,
        audioObjects,
        audioOutObjects;

    QList<AUDIO::AudioBuffer*>
        audioIns,
        audioOuts;

#ifndef MO_DISABLE_SERVER
    UdpAudioConnection * udpOutput;
#endif
#ifndef MO_DISABLE_CLIENT
    UdpAudioConnection * udpInput;
#endif
};

ObjectDspPath::ObjectDspPath()
    : p_    (new Private(this))
{
}

ObjectDspPath::~ObjectDspPath()
{
    delete p_;
}

const AUDIO::Configuration & ObjectDspPath::config() const
{
    return p_->conf;
}

const QList<AUDIO::AudioBuffer*> & ObjectDspPath::audioInputs()
{
    return p_->audioIns;
}

const QList<AUDIO::AudioBuffer*> & ObjectDspPath::audioOutputs()
{
    return p_->audioOuts;
}

void ObjectDspPath::createPath(Scene *scene, const AUDIO::Configuration& conf, uint thread)
{
    p_->thread = thread;
    p_->conf = conf;
    p_->createPath(scene);
}

void ObjectDspPath::preparePath()
{
    MO_DEBUG("ObjectDspPath::preparePath()");

    for (Private::ObjectBuffer * b : p_->audioObjects)
        p_->prepareUdpOutputs(b);
}

void ObjectDspPath::calcTransformations(SamplePos pos)
{
    for (Private::ObjectBuffer * b : p_->transformationObjects)
    {
        if (b->parentMatrix)
        {
            // copy from parent
            TransformationBuffer::copy(b->parentMatrix, &b->calcMatrix);
            // apply transform for one sample block
            for (SamplePos i = 0; i < p_->conf.bufferSize(); ++i)
                b->object->calculateTransformation(b->calcMatrix.transformation(i),
                                                   p_->conf.sampleRateInv() * (pos + i),
                                                   p_->thread);
        }
    }
}

void ObjectDspPath::calcAudio(SamplePos pos)
{
#ifndef MO_DISABLE_SERVER
    auto serv = serverEngine().isRunning();
#else
    bool serv = false;
#endif

    // ----------- process audio objects ---------------

    // process audio objects
    for (Private::ObjectBuffer * b : p_->audioObjects)
    {
#ifndef MO_DISABLE_CLIENT
        // unless they are sent from server
        if (!b->udpInput)
        {
#endif
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

            // stream to udp clients
#ifndef MO_DISABLE_SERVER
            if (serv && b->udpOutput)
                for (AUDIO::AudioBuffer * ab : b->udpOutputBuffers)
                    b->udpOutput->sendAudioBuffer(ab, pos);
#endif
#ifndef MO_DISABLE_CLIENT
        }
        // get module output from udp streams
        else
        {
            // xxx right now it's done by UdpAudioConnection on receive of packets
        }
#endif
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


std::ostream& ObjectDspPath::dump(std::ostream & out) const
{
    out << "dsp-graph (" << p_->conf << ")\n"
           "transformation objects:";
    for (auto o : p_->transformationObjects)
    {
        out << " " << o->object->name();
        if (o->posParent)
            out << "(" << o->posParent->object->name() << ")";
    }

    out << "\nmicrophone objects:";
    for (auto o : p_->microphoneObjects)
    {
        out << " " << o->object->name();
#ifdef MO_DO_DEBUG
        out << "(" << o->matrixParent->object->name() << ")";
#endif
        out << "(" << o->microphoneInputSoundSources.size() << " ins)";
    }

    out << "\nsoundsource objects:";
    for (auto o : p_->soundsourceObjects)
    {
        out << " " << o->object->name();
#ifdef MO_DO_DEBUG
        out << "(" << o->matrixParent->object->name() << ")";
#endif
    }

    out << "\naudio objects:";
    for (auto o : p_->audioObjects)
    {
        out << " " << o->object->name();
    }

    out << "\naudio-out objects:";
    for (auto o : p_->audioOutObjects)
    {
        out << " " << o->object->name();
    }

    out << "\naudio objects detail:\n";
    for (Private::ObjectBuffer * o : p_->audioObjects)
    {
        auto ao = static_cast<AudioObject*>(o->object);
        out << o->object->name() << " "
            << (o->udpInput ? "UDP " : "")
            << o->audioInputs.size() << "/" << o->audioOutputs.size()
            << " (" << ao->numAudioInputs() << "/" << ao->numAudioOutputs() << ")\n";

        for (auto b : o->audioInputs)
            out << "  in " << b << "\n";

        for (auto & mix : o->audioInputMix)
        {
            out << "  mix ";
            for (auto b : mix.inputs)
                out << " " << b;
            out << "\n";
        }

        for (auto b : o->audioOutputs)
        {
            out << " out " << b;
            if (o->udpOutput && o->udpOutputBuffers.contains(b))
                out << " UDP OUT";
            if (o->udpInput && o->udpInputBuffers.contains(b))
                out << " UDP IN";
            out << "\n";
        }
    }

    out << std::endl;
    return out;
}


void ObjectDspPath::Private::clear()
{
    for (auto b : audioOuts)
        delete b;
    for (auto b : audioIns)
        delete b;

    scene = 0;
    objects.clear();
    transformationObjects.clear();
    microphoneObjects.clear();
    soundsourceObjects.clear();
    audioObjects.clear();
    audioIns.clear();
    audioOuts.clear();
    audioOutObjects.clear();
    audioToModulator.clear();
    if (udpInput)
        udpInput->clear();
    if (udpOutput)
        udpOutput->clear();
}

void ObjectDspPath::Private::createPath(Scene * s)
{
    clear();
    scene = s;

    // little hack until concept for microphone routing
    uint numGlobalMicrophone = 0;

    // -------------------- system io ---------------------------

    // system-in-buffers
    for (uint i = 0; i < conf.numChannelsIn(); ++i)
        audioIns.push_back( new AUDIO::AudioBuffer(conf.bufferSize()) );

    // system-out-buffers
    for (uint i = 0; i < conf.numChannelsOut(); ++i)
        audioOuts.push_back( new AUDIO::AudioBuffer(conf.bufferSize()) );


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

    // Make a lookup table of all audio outputs that are
    // mapped to modulator inputs
    // [So we know which audio objects to process even if they are
    //  not connected to other audio objects]
    for (Modulator * m : all_modulators)
        if (m->isAudioToFloatConverter())
            addAudioToModulator(m);


    // ---- find all objects that need translation calculated ---

    // make a tree of all audio-relevant translation objects
    // [the copy function will always copy the root/scene object
    //  so it can serve as the root transformation (identity)]
    auto audioPosTree = tree->copy(false, [](Object * o)
    {
        return (o->isAudioRelevant() && o->hasTransformationObjects());
    });

#ifdef MO_GRAPH_DEBUG
    MO_DEBUG("audioPosTree:");
    audioPosTree->dumpTree(std::cout);
#endif

    // create object buffers and assign position parents from tree
    audioPosTree->forEachNode([this](ObjectTreeNode * n)
    {
        ObjectBuffer * b = getObjectBuffer(n->object());
        if (n->parent())
        {
            b->posParent = getObjectBuffer(n->parent()->object());
            b->parentMatrix = &b->posParent->calcMatrix;
        }
        // only audio-relevant objects with transformations
        // and the scene object get this matrix resized
        b->calcMatrix.resize(conf.bufferSize());

        transformationObjects.append(b);
    });


    // ---------------- get all audio processors ----------------

    // copy audio connections to DirectedGraph helper
    DirectedGraph<AudioObject*> dspgraph;
    // this only inserts objects that have audio connections
    // all others are ignored.
    for (auto i : *scene->audioConnections())
        dspgraph.addEdge(i->from(), i->to());

    // create linear list in correct order
    QList<AudioObject*> audioObjectList;
    {
        dspgraph.makeLinear(audioObjectList);

        // also add the dsp objects that don't have audio-outputs
        // but modulator-outputs
        // [can be safely added to the back because they don't depend on one another]
        for (Modulator * m : all_modulators)
            if (m->isAudioToFloatConverter())
                // if the graph does not contain it already
                if (!dspgraph.node(static_cast<AudioObject*>(m->modulator())))
                    audioObjectList << static_cast<AudioObject*>(m->modulator());
    }

    // initialize buffers for the objects in the linear list
    for (AudioObject * o : audioObjectList)
    {
        // for clients, init only the audio -> modulator objects
        if (!isClient() || hasAudioToModulator(o))
        {
            /** @todo this is a problem.
                Changing the number of channels touches through to the gui */
            // update sys io objects' channels
            if (auto ao = qobject_cast<AudioInAO*>(o))
                ao->setNumberAudioInputsOutputs(audioIns.size());
            if (auto ao = qobject_cast<AudioOutAO*>(o))
                ao->setNumberAudioInputsOutputs(audioOuts.size());

            // ObjectBuffer for each audio object
            auto b = getObjectBuffer(o);
            audioObjects.append( b );
/*
#ifndef MO_DISABLE_CLIENT
            if (isClient())
            {
                // link to input streams / flag as client audio object
                b->udpInput = getUdpInput();
            }
#endif
#ifndef MO_DISABLE_SERVER
            // link to output streams / flag as server audio object
            if (isServer() && hasAudioToModulator(o)
                //&& serverEngine().isRunning()
                )
            {
                b->udpOutput = getUdpOutput();
            }
#endif
*/
            // create and link buffers
            prepareAudioInputBuffers(b);
            prepareAudioOutputBuffers(b);

            // tell object
            o->setAudioBuffersBase(thread, conf.bufferSize(), b->audioInputs, b->audioOutputs);
        }
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


}




void ObjectDspPath::Private::prepareAudioInputBuffers(ObjectBuffer * buf)
{
    MO_ASSERT(dynamic_cast<AudioObject*>(buf->object),
              "invalid object for audio input preparation");

    AudioObject * o = static_cast<AudioObject*>(buf->object);

    // system-audio-in object?
    if (qobject_cast<AudioInAO*>(o))
    {
        // copy the list of sys-inputs
        buf->audioInputs = audioIns;

        return;
    }

    // no need for client audio objects
#ifndef MO_DISABLE_CLIENT
    if (buf->udpInput)
        return;
#endif

    // get input connections
    auto ins = scene->audioConnections()->getInputs(o);

    int numChannels = o->numAudioInputs();

    // determine from inputs
    if (numChannels < 0)
        for (auto c : ins)
            numChannels = std::max(numChannels, (int)c->inputChannel() + 1);

    // for each channel
    for (int i = 0; i < numChannels; ++i)
    {
        // collect all inputs
        QList<AUDIO::AudioBuffer*> inputs;
        for (AudioObjectConnection * c : ins)
        if ((int)c->inputChannel() == i)
        {
            // find input module
            auto inb = getObjectBuffer(c->from());
#if (0)
            MO_ASSERT((int)c->outputChannel() < inb->audioOutputs.size(),
                      "connection requires illegal channel "
                      << c->outputChannel() << "/" << inb->audioOutputs.size());
#else
            if ((int)c->outputChannel() >= inb->audioOutputs.size())
                MO_WARNING("connection requires illegal channel "
                           << c->outputChannel() << "/" << inb->audioOutputs.size())
            else
#endif
            inputs.push_back( inb->audioOutputs[c->outputChannel()] );
        }

        // empty slot
        if (inputs.isEmpty())
            buf->audioInputs.push_back( 0 );
        else
        // pass through
        if (inputs.size() == 1)
            buf->audioInputs.push_back( inputs.first() );
        else
        // create mix step
        {
            InputMixStep mix(new AUDIO::AudioBuffer(conf.bufferSize()));
            mix.inputs = inputs;
            buf->audioInputMix.append( mix );

            // and wire it's output to our input
            buf->audioInputs.push_back( mix.buf );
        }
    }
}



void ObjectDspPath::Private::prepareAudioOutputBuffers(ObjectBuffer * buf)
{
    MO_ASSERT(dynamic_cast<AudioObject*>(buf->object),
              "invalid object for audio output preparation");

    AudioObject * ao = static_cast<AudioObject*>(buf->object);

    // special system-out object?
    AudioOutAO * oout = qobject_cast<AudioOutAO*>(ao);

    // prepare outputs
    // (create an entry in ObjectBuffer::audioOutputs for each output
    //  and set unused outputs to NULL)
    if (!oout)
    {
        auto outs = scene->audioConnections()->getOutputs(ao);
        for (uint i = 0; i < ao->numAudioOutputs(); ++i)
        {
            // links (also) to a modulator?
            const bool isModOut = hasAudioToModulator(ao, i);

            // number of buffers in output (dsp block size times this number)
            uint numBlocks
                    // at least 1 second for all (FrontDisplayItems might need it)
                    /** @todo make this dynamic for actual displayItems */
                         = (conf.sampleRate() * 1) / conf.bufferSize() + 1;

            if (isModOut)
                // at least three seconds of buffer time for mod-outs
                /** @todo make configurable */
                numBlocks = (conf.sampleRate() * 3) / conf.bufferSize() + 1;

            AUDIO::AudioBuffer * audiobuf = 0;

            for (AudioObjectConnection * c : outs)
                // for each audio or mod-out connection
                if (c->outputChannel() == i || isModOut)
                {
                    // skip audio-only outs for clients
#ifndef MO_DISABLE_CLIENT
                    if (buf->udpInput && !isModOut)
                        continue;
#endif
                    audiobuf = new AUDIO::AudioBuffer(conf.bufferSize(), numBlocks);
                    break;
                }

            // in case no audio-out but still mod out
            if (!audiobuf && isModOut)
            {
                audiobuf = new AUDIO::AudioBuffer(conf.bufferSize(), numBlocks);
            }

            // otherwise keep NULL buffer
            buf->audioOutputs.push_back( audiobuf );
/*
            // link buffer to udp audio-input stream
            // [so streams can fill the output buffers of the object]
#ifndef MO_DISABLE_CLIENT
            if (buf->udpInput && audiobuf)
            {
                buf->udpInput->addBuffer(audiobuf, ao, i);
                buf->udpInputBuffers.append(audiobuf);
            }
#endif
            // link to udp audio-out stream
            // for outputs that clients need
#ifndef MO_DISABLE_SERVER
            if (buf->udpOutput && audiobuf && isModOut)
            {
                buf->udpOutput->addBuffer(audiobuf, ao, i);
                buf->udpOutputBuffers.append(audiobuf);
            }
#endif
*/
        }
    }
    // perpare outputs of system-out object
    // [invisible to the user]
    else
    {
        // find highest input channel
        auto ins = scene->audioConnections()->getInputs(ao);
        uint num = 0;
        for (auto c : ins)
            num = std::max(num, c->inputChannel() + 1);
        // but limit to max system-outputs
        num = std::min(num, conf.numChannelsOut());
        // prepare an output for each input
        for (uint i=0; i<num; ++i)
        {
            bool used = false;
            for (AudioObjectConnection * c : ins)
            if (c->inputChannel() == i)
            {
                auto audiobuf = new AUDIO::AudioBuffer(conf.bufferSize());
                buf->audioOutputs.push_back( audiobuf );
                used = true;
                break;
            }
            if (!used)
                buf->audioOutputs.push_back( 0 );
        }

        // remember audio-out objects separately
        audioOutObjects.push_back( buf );
    }
}

void ObjectDspPath::Private::prepareUdpOutputs(ObjectBuffer * buf)
{
    MO_ASSERT(dynamic_cast<AudioObject*>(buf->object),
              "invalid object for udp output preparation");

    AudioObject * ao = static_cast<AudioObject*>(buf->object);

    // special system-out object?
    AudioOutAO * oout = qobject_cast<AudioOutAO*>(ao);

    // prepare outputs
    // (create an entry in ObjectBuffer::audioOutputs for each output
    //  and set unused outputs to NULL)
    if (!oout)
    {
        auto outs = scene->audioConnections()->getOutputs(ao);
        for (uint i = 0; i < ao->numAudioOutputs(); ++i)
        {
            // get prepared output buffer
            auto audiobuf = ao->audioOutputs(thread).at(i);
            // just an output without connection??
            if (!audiobuf)
                continue;

            // audio channel needed by a modulator?
            bool isModOut = hasAudioToModulator(ao, i);

            //MO_ASSERT(audiobuf, "unprepared audio buffer in DspPath::prepareUdpOutputs ("
            //          << ao->namePath() << ", out " << i << ")");

#ifndef MO_DISABLE_CLIENT
            // link buffer to udp audio-input stream
            // [so streams can fill the output buffers of the object]
            if (isModOut && isClient() && !buf->udpInput)
            {
                buf->udpInput = getUdpInput();
                buf->udpInput->addBuffer(audiobuf, ao, i);
                buf->udpInputBuffers.append(audiobuf);
                buf->udpInput->openForRead();
            }
#endif

#ifndef MO_DISABLE_SERVER
            // link to udp audio-out stream
            // for outputs that clients need
            if (isModOut && isServer() && !buf->udpOutput)
            {
                buf->udpOutput = getUdpOutput();
                buf->udpOutput->addBuffer(audiobuf, ao, i);
                buf->udpOutputBuffers.append(audiobuf);
            }
#endif
        }
    }
}





ObjectDspPath::Private::ObjectBuffer * ObjectDspPath::Private::getParentTransformationObject(ObjectBuffer * buf)
{
    if (buf->calcMatrix.bufferSize())
        return buf;

    Object * o = buf->object->parentObject();
    buf = findObjectBuffer(o);
    while (!buf || !buf->calcMatrix.bufferSize())
    {
        o = o->parentObject();
        buf = findObjectBuffer(o);
    }
    MO_ASSERT(buf, "no transformation parent found");
    return buf;
}

void ObjectDspPath::Private::assignMatrix(ObjectBuffer * b)
{
    ObjectBuffer * buf = getParentTransformationObject(b);
    b->matrix = &buf->calcMatrix;
#ifdef MO_DO_DEBUG
    b->matrixParent = buf;
#endif
}

#ifndef MO_DISABLE_CLIENT
UdpAudioConnection * ObjectDspPath::Private::getUdpInput()
{
    MO_ASSERT(isClient(), "wrong request");
    if (!udpInput)
    {
        MO_DEBUG("ObjectDspPath: creating udp input object");
        udpInput = new UdpAudioConnection();
    }
    return udpInput;
}
#endif

#ifndef MO_DISABLE_SERVER
UdpAudioConnection * ObjectDspPath::Private::getUdpOutput()
{
    MO_ASSERT(!isClient(), "wrong request");
    if (!udpOutput)
    {
        MO_DEBUG("ObjectDspPath: creating udp output object");
        udpOutput = //serverEngine().getAudioOutStream();
                    new UdpAudioConnection();
    }
    return udpOutput;
}
#endif


void ObjectDspPath::Private::addAudioToModulator(Modulator * m)
{
    MO_ASSERT(m->isAudioToFloatConverter(), "");
    AudioObject * ao = static_cast<AudioObject*>(m->modulator());
    if (!ao)
    {
        MO_WARNING("unassigned Modulator in ObjectDspPath::addAudioToModulator()");
        return;
    }

    auto i = audioToModulator.find(ao);
    if (i == audioToModulator.end())
    {
        // create new entry
        QSet<uint> cons;
        cons.insert( m->getAudioOutputChannel() );
        audioToModulator.insert( ao, cons );
    }
    else
    {
        // add to exisiting entry
        i.value().insert( m->getAudioOutputChannel() );
    }
}

bool ObjectDspPath::Private::hasAudioToModulator(AudioObject * ao)
{
    auto i = audioToModulator.find(ao);
    return i != audioToModulator.end();
}

bool ObjectDspPath::Private::hasAudioToModulator(AudioObject * ao, uint channel)
{
    auto i = audioToModulator.find(ao);
    if (i != audioToModulator.end())
    {
        return i.value().contains(channel);
    }
    return false;
}

ObjectDspPath::Private::ObjectBuffer * ObjectDspPath::Private::getObjectBuffer(Object * o)
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

ObjectDspPath::Private::ObjectBuffer * ObjectDspPath::Private::findObjectBuffer(Object * o)
{
    auto i = objects.find(o);
    if (i != objects.end())
        return i->second.get();
    else
        return 0;
}

void ObjectDspPath::Private::createObjectBuffers(const QList<Object *> &input, QList<ObjectBuffer *> &output)
{
    output.clear();
    for (auto o : input)
        output.append( getObjectBuffer(o) );
}

} // namespace MO
