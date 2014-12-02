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
#include "object/util/objecttree.h"
#include "object/audio/objectdsppath.h"
#include "object/util/audioobjectconnections.h"
#include "audio/tool/audiobuffer.h"
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
        /// Audio output buffers
        std::vector<AUDIO::AudioBuffer*>
            audioOutputs;

        void initMatrix(int s)
        {
            matrix.resize(s);
            for (auto & m : matrix)
                m = Mat4(1.);
        }
    };

    Private(ObjectDspPath * path)
        : path          (path),
          scene         (0),
          sampleRate    (1),
          bufferSize    (0),
          invSampleRate (1.)
    { }


    void createPath(Scene *);
    ObjectBuffer * getObjectBuffer(Object *); ///< always returns a valid buffer


    ObjectDspPath * path;
    Scene * scene;
    uint sampleRate, bufferSize;
    Double invSampleRate;

    // all relevant objects
    std::map<Object *, std::shared_ptr<ObjectBuffer>> objects;
    /** Transforms a list of objects to it's coresponding buffers */
    void createObjectBuffers(const QList<Object*> & input, QList<ObjectBuffer*>& output);

    QList<ObjectBuffer*>
        transformationObjects,
        soundsourceObjects,
        microphoneObjects,
        audioObjects;
};

ObjectDspPath::ObjectDspPath()
    : p_    (new Private(this))
{
}

ObjectDspPath::~ObjectDspPath()
{
    delete p_;
}

uint ObjectDspPath::sampleRate() const
{
    return p_->sampleRate;
}

uint ObjectDspPath::bufferSize() const
{
    return p_->bufferSize;
}


void ObjectDspPath::createPath(Scene *scene, uint sampleRate, uint bufferSize)
{
    p_->sampleRate = sampleRate;
    p_->invSampleRate = 1.0 / std::max(1u, p_->sampleRate);
    p_->bufferSize = bufferSize;
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
            for (SamplePos i = 0; i < bufferSize(); ++i)
                b->object->calculateTransformation(b->matrix[i],
                                                   p_->invSampleRate * (pos + i),
                                                   thread);
        }
    }
}

void ObjectDspPath::calcAudio(SamplePos pos, uint thread)
{
    /*for (Private::ObjectBuffer * b : p_->soundsourceObjects)
    {

        //b->object->performAudioBlock(pos, thread);

    }*/
}


std::ostream& ObjectDspPath::dump(std::ostream & out) const
{
    out << "dsp-graph (rate=" << sampleRate() << ", block=" << bufferSize() << ")\n"
           "transformation objects:";
    for (auto o : p_->transformationObjects)
    {
        out << " " << o->object->name();
        if (o->posParent)
            out << "(" << o->posParent->object->name() << ")";
    }

    out << "\naudio objects:";
    for (auto o : p_->audioObjects)
    {
        out << " " << o->object->name();
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


    out << std::endl;
    return out;
}



void ObjectDspPath::Private::createPath(Scene * s)
{
    scene = s;
    objects.clear();
    transformationObjects.clear();
    microphoneObjects.clear();
    soundsourceObjects.clear();
    audioObjects.clear();

    // convert to new tree structure
    auto tree = get_object_tree(scene);
    std::auto_ptr<ObjectTreeNode> tree_delete(tree);

    // ---- find all objects that need translation calculated ---

    // make a tree of all audio-relevant translation objects
    auto audioPosTree = tree->copy(false, [](Object * o)
    {
        return (o->isAudioRelevant() && o->hasTransformationObjects());
    });
    audioPosTree->dumpTree(std::cout);

    // create object buffers and assign position parents from tree
    audioPosTree->forEachNode([this](ObjectTreeNode * n)
    {
        ObjectBuffer * b = getObjectBuffer(n->object());
        if (n->parent())
        {
            b->posParent = getObjectBuffer(n->parent()->object());
            b->parentMatrix = &b->posParent->matrix;
        }
        b->initMatrix(bufferSize);

        transformationObjects.append(b);
    });

    // --- get all audio processors ---

    DirectedGraph<AudioObject*> dspgraph;
    for (auto i : *scene->audioConnections())
        dspgraph.addEdge(i->from(), i->to());

    QList<AudioObject*> audioObjectList;
    dspgraph.makeLinear(audioObjectList);

    for (auto o : audioObjectList)
    {
        // ObjectBuffer for each audio object
        auto b = getObjectBuffer(o);
        audioObjects.append( b );
        // prepare outputs
        auto outs = scene->audioConnections()->getOutputs(o);
        for (AudioObjectConnection * c : outs)
        {
            auto buf = new AUDIO::AudioBuffer(bufferSize);
            b->audioOutputs.push_back( buf );
        }
        // prepare inputs

    }

    // --- get all soundsource objects ---

    QList<Object*> soundsources;
    tree->makeLinear(soundsources, [](const Object*o)
    {
        return !o->audioSources().isEmpty();
    });
    createObjectBuffers(soundsources, soundsourceObjects);


    // --- get all microphone objects ---

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
