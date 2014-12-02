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
#include "object/util/objecttree.h"
#include "object/util/objectdsppath.h"
#include "object/util/audioobjectconnections.h"
#include "audio/tool/audiobuffer.h"
#include "audio/configuration.h"
#include "graph/directedgraph.h"
#include "io/log.h"

namespace MO {

class ObjectDspPath::Private
{
public:

    /** Wrapper for all objects
        with temporary calculation space */
    struct ObjectBuffer
    {
        ObjectBuffer()
            : object(0), posParent(0),
              parentMatrix(0)
        { }

        ~ObjectBuffer()
        {
            for (auto b : audioOutputs)
                delete b;
        }

        /// Object associated to this buffer
        Object * object;
        /// The translation parent
        ObjectBuffer * posParent;
        /// Calculated matrix
        std::vector<Mat4> matrix,
        /// Link to parent translation
            *parentMatrix;
        QList<AUDIO::AudioBuffer*>
        /// Audio output buffers (memory managed)
            audioOutputs,
        /// Links connected audio output buffers
            audioInputs;


        void initMatrix(int s)
        {
            matrix.resize(s);
            for (auto & m : matrix)
                m = Mat4(1.);
        }
    };

    Private(ObjectDspPath * path)
        : path          (path),
          scene         (0)
    { }

    ~Private()
    {
        clear();
    }

    void clear();
    void createPath(Scene *);
    ObjectBuffer * getObjectBuffer(Object *); ///< always returns a valid buffer


    ObjectDspPath * path;
    Scene * scene;
    AUDIO::Configuration conf;

    // all relevant objects
    std::map<Object *, std::shared_ptr<ObjectBuffer>> objects;
    /** Transforms a list of objects to it's coresponding buffers */
    void createObjectBuffers(const QList<Object*> & input, QList<ObjectBuffer*>& output);

    QList<ObjectBuffer*>
        transformationObjects,
        soundsourceObjects,
        microphoneObjects,
        audioObjects,
        audioOutObjects;

    QList<AUDIO::AudioBuffer*> audioOut;
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

const QList<AUDIO::AudioBuffer*> & ObjectDspPath::audioOutputs()
{
    return p_->audioOut;
}

void ObjectDspPath::createPath(Scene *scene, const AUDIO::Configuration& conf)
{
    p_->conf = conf;
    p_->createPath(scene);
}

void ObjectDspPath::calcTransformations(SamplePos pos, uint thread)
{
    for (Private::ObjectBuffer * b : p_->transformationObjects)
    {
        if (b->parentMatrix)
        {
            // copy from parent
            b->matrix = *b->parentMatrix;
            // apply transform for one sample block
            for (SamplePos i = 0; i < p_->conf.bufferSize(); ++i)
                b->object->calculateTransformation(b->matrix[i],
                                                   p_->conf.sampleRateInv() * (pos + i),
                                                   thread);
        }
    }
}

void ObjectDspPath::calcAudio(SamplePos pos, uint thread)
{
    // clear system audio outputs
    for (AUDIO::AudioBuffer * buf : p_->audioOut)
        buf->writeNullBlock();

    // process audio objects
    for (Private::ObjectBuffer * b : p_->audioObjects)
    {
        auto ao = static_cast<AudioObject*>(b->object);
        // process AudioObject
        ao->processAudioBase(
                    b->audioInputs, b->audioOutputs,
                    p_->conf.bufferSize(), pos, thread);

        // forward buffers
        for (AUDIO::AudioBuffer * buf : b->audioOutputs)
            if (buf)
                buf->nextBlock();
    }

    // mix into system audio outputs
    for (Private::ObjectBuffer * b : p_->audioOutObjects)
        AUDIO::AudioBuffer::mix(p_->audioOut, b->audioOutputs);

    /*for (Private::ObjectBuffer * b : p_->soundsourceObjects)
    {

        //b->object->performAudioBlock(pos, thread);

    }*/
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
    }

    out << "\nsoundsource objects:";
    for (auto o : p_->soundsourceObjects)
    {
        out << " " << o->object->name();
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
    for (auto o : p_->audioObjects)
    {
        auto ao = dynamic_cast<AudioObject*>(o->object);
        out << o->object->name() << "(" << ao->numAudioOutputs() << " desired outs)\n";
        for (auto b : o->audioInputs)
            out << " " << b << "->\n";
        for (auto b : o->audioOutputs)
            out << " ->" << b << "\n";
    }

    out << std::endl;
    return out;
}


void ObjectDspPath::Private::clear()
{
    for (auto b : audioOut)
        delete b;

    scene = 0;
    objects.clear();
    transformationObjects.clear();
    microphoneObjects.clear();
    soundsourceObjects.clear();
    audioObjects.clear();
    audioOut.clear();
    audioOutObjects.clear();
}

void ObjectDspPath::Private::createPath(Scene * s)
{
    clear();
    scene = s;

    // -------------------- system io ---------------------------

    // system-out-buffers
    for (uint i = 0; i < conf.numChannelsOut(); ++i)
        audioOut.push_back( new AUDIO::AudioBuffer(conf.bufferSize()) );


    // ---------------- analyze object tree ---------------------

    // convert to new tree structure
    auto tree = get_object_tree(scene);
    std::auto_ptr<ObjectTreeNode> tree_delete(tree);


    // ---- find all objects that need translation calculated ---

    // make a tree of all audio-relevant translation objects
    auto audioPosTree = tree->copy(false, [](Object * o)
    {
        return (o->isAudioRelevant() && o->hasTransformationObjects());
    });
#ifdef MO_GRAPH_DEBUG
    audioPosTree->dumpTree(std::cout);
#endif

    // create object buffers and assign position parents from tree
    audioPosTree->forEachNode([this](ObjectTreeNode * n)
    {
        ObjectBuffer * b = getObjectBuffer(n->object());
        if (n->parent())
        {
            b->posParent = getObjectBuffer(n->parent()->object());
            b->parentMatrix = &b->posParent->matrix;
        }
        b->initMatrix(conf.bufferSize());

        transformationObjects.append(b);
    });


    // ---------------- get all audio processors ----------------

    DirectedGraph<AudioObject*> dspgraph;
    for (auto i : *scene->audioConnections())
        dspgraph.addEdge(i->from(), i->to());

    QList<AudioObject*> audioObjectList;
    dspgraph.makeLinear(audioObjectList);

    for (AudioObject * o : audioObjectList)
    {
        // ObjectBuffer for each audio object
        auto b = getObjectBuffer(o);
        audioObjects.append( b );

        // special system-out object?
        AudioOutAO * oout = qobject_cast<AudioOutAO*>(o);

        // prepare outputs
        // (create an entry in ObjectBuffer::audioOutputs for each output
        //  and set unused outputs to NULL)
        if (!oout)
        {
            auto outs = scene->audioConnections()->getOutputs(o);
            for (uint i = 0; i < o->numAudioOutputs(); ++i)
            {
                bool used = false;
                for (AudioObjectConnection * c : outs)
                if (c->outputChannel() == i)
                {
                    auto buf = new AUDIO::AudioBuffer(conf.bufferSize());
                    b->audioOutputs.push_back( buf );
                    used = true;
                    break;
                }
                if (!used)
                    b->audioOutputs.push_back( 0 );
            }
        }
        // perpare outputs of system-out object
        else
        {
            // find highest input channel
            auto ins = scene->audioConnections()->getInputs(o);
            uint num = 0;
            for (auto c : ins)
                num = std::max(num, c->inputChannel());
            // but limit to max system-outputs
            num = std::min(num, conf.numChannelsOut());
            // prepare an output for each input
            for (uint i=0; i<num; ++i)
            {
                bool used = false;
                for (AudioObjectConnection * c : ins)
                if (c->inputChannel() == i)
                {
                    auto buf = new AUDIO::AudioBuffer(conf.bufferSize());
                    b->audioOutputs.push_back( buf );
                    used = true;
                    break;
                }
                if (!used)
                    b->audioOutputs.push_back( 0 );
            }

            // remember audio-out objects separately
            audioOutObjects.push_back( b );
        }

        // prepare inputs
        auto ins = scene->audioConnections()->getInputs(o);
        for (AudioObjectConnection * c : ins)
        {
            // find input module
            auto inb = getObjectBuffer(c->from());
            MO_ASSERT((int)c->outputChannel() < inb->audioOutputs.size(), "");

            b->audioInputs.push_back( inb->audioOutputs[c->outputChannel()] );
        }
    }


    // ----------- get all soundsource objects ------------------

    QList<Object*> soundsources;
    tree->makeLinear(soundsources, [](const Object*o)
    {
        return !o->audioSources().isEmpty();
    });
    createObjectBuffers(soundsources, soundsourceObjects);


    // ----------- get all microphone objects -------------------

    QList<Object*> microphones;
    tree->makeLinear(microphones, [](const Object*o)
    {
        return !o->microphones().isEmpty();
    });
    createObjectBuffers(microphones, microphoneObjects);


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

void ObjectDspPath::Private::createObjectBuffers(const QList<Object *> &input, QList<ObjectBuffer *> &output)
{
    output.clear();
    for (auto o : input)
        output.append( getObjectBuffer(o) );
}

} // namespace MO
