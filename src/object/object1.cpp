/** @file object1.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 30.10.2014</p>
*/

#include "object1.h"
#include "io/datastream.h"
#include "io/log.h"
#include "param/parameterselect.h"

namespace MO {

class Object1::PrivateObject
{
public:
    PrivateObject()
        : node                  (0),
          canBeDeleted          (true),
          childrenHaveChanged   (false),
          parentActivityScope   (AS_ON),
          currentActivityScope  (AS_ON),
          numberThreads         (1),
          bufferSize            (1),
          sampleRate            (44100),
          sampleRateInv         (1.0/44100.0),
          paramActiveScope      (0)

    { }

    // ------------ member ---------------

    TreeNode<Object1*> * node;

    // ------------ properties ---------------

    QString
        idName,
        name;

    bool canBeDeleted,
         childrenHaveChanged;

    ActivityScope
    /** activity scope passed down from parents */
        parentActivityScope,
    /** current requested activity scope */
        currentActivityScope;

    // ---------- threads and blocksize ------

    uint numberThreads;
    std::vector<uint> bufferSize;

    uint sampleRate;
    Double sampleRateInv;

    // ----------- parameter -----------------

    QList<Parameter*> parameters;
    QString currentParameterGroupId,
            currentParameterGroupName;

    // --------- default parameters ----------

    ParameterSelect * paramActiveScope;

};



Object1::Object1()
    : p_o_  (new PrivateObject())
{

}

Object1::~Object1()
{
    for (auto p : p_o_->parameters)
        delete p;

    delete p_o_;
}



void Object1::serialize(IO::DataStream & io) const
{
    io.writeHeader("obj", 1);

    io << p_o_->canBeDeleted;
}

void Object1::deserialize(IO::DataStream & io)
{
    io.readHeader("obj", 1);

    io >> p_o_->canBeDeleted;
}



// ################################ getter #############################################

int Object1::objectPriority(const Object1 * o)
{
    if (o->isTransformation())
        return 3;
    if (o->isModulatorObject())
        return 2;
    if (o->isAudioUnit())
        return 1;
    return 0;
}

const QString& Object1::idName() const
{
    return p_o_->idName;
}

const QString& Object1::name() const
{
    return p_o_->name;
}

QString Object1::namePath() const
{
    Object1 * p = parentObject();
    if (!p)
        return "/" + name();

    QString path;
    while (p)
    {
        path.prepend("/" + p->name());
        p = p->parentObject();
    }

    return "/" + path + "/" + name();
}

QString Object1::idNamePath() const
{
    Object1 * p = parentObject();
    if (!p)
        return "/" + idName();

    QString path;
    while (p)
    {
        path.prepend("/" + p->idName());
        p = p->parentObject();
    }

    return "/" + path + "/" + idName();
}

uint Object1::numberThreads() const
{
    return p_o_->numberThreads;
}

bool Object1::isAudioRelevant() const
{
    return false;
}

bool Object1::canBeDeleted() const
{
    return p_o_->canBeDeleted;
}


bool Object1::verifyNumberThreads(uint num)
{
    if (p_o_->numberThreads != num)
        return false;

    return true;
}

// ---------- activity (scope) ----------------

Object1::ActivityScope Object1::activityScope() const
{
    if (p_o_->paramActiveScope)
        return (ActivityScope)(
                    p_o_->paramActiveScope->baseValue() & p_o_->parentActivityScope);
    else
        return p_o_->parentActivityScope;
}

Object1::ActivityScope Object1::currentActivityScope() const
{
    return p_o_->currentActivityScope;
}

bool Object1::active(Double /*time*/, uint /*thread*/) const
{
    // XXX active parameter not there yet
    return activityScope() & p_o_->currentActivityScope;
}

bool Object1::activeAtAll() const
{
    return activityScope() & p_o_->currentActivityScope;
}







// #################################### setter #########################################

void Object1::setName(const QString & n)
{
    p_o_->name = n;
}

void Object1::p_set_id_(const QString& id)
{
    p_o_->idName = id;
}

void Object1::p_set_node_(Node * n)
{
    p_o_->node = n;
}

void Object1::setNumberThreads(uint num)
{
    MO_DEBUG_TREE("Object('" << idName() << "')::setNumberThreads(" << num << ")");

    p_o_->numberThreads += num;

    //p_o_->transformation.resize(num);
    p_o_->bufferSize.resize(num);
}

void Object1::setCurrentActivityScope(ActivityScope scope)
{
    p_o_->currentActivityScope = scope;

    for (auto n : node()->children())
        n->object()->setCurrentActivityScope(scope);
}


// ################################### tree stuff ######################################

// ---------- tree getter --------------------

TreeNode<Object1*> * Object1::node() const
{
    return p_o_->node;
}

const Object1 * Object1::rootObject() const
{
    auto n = node()->root();
    return n ? n->object() : 0;
}

Object1 * Object1::rootObject()
{
    auto n = node()->root();
    return n ? n->object() : 0;
}

const Scene * Object1::sceneObject() const
{
    //return dynamic_cast<const Scene*>(rootObject());
    return 0;
}

Scene * Object1::sceneObject()
{
    //return dynamic_cast<Scene*>(rootObject());
    return 0;
}

Object1 * Object1::parentObject() const
{
    return p_o_->node ? p_o_->node->parent()->object() : 0;
}

bool Object1::canHaveChildren(Type t) const
{
    // dummy can contain/be added to everything
    if (t == T_DUMMY || type() == T_DUMMY)
        return true;

    // XXX for now: ModulatorObjects can be anywhere
    if (t & TG_MODULATOR_OBJECT)
        return true;

    // XXX test123: Let's place sequences everywhere
    if (t & TG_SEQUENCE)
        return true;

    // Clips belong into ClipContainer ...
    if (t == T_CLIP)
        return type() == T_CLIP_CONTAINER;

    // and nothing else
    if (type() == T_CLIP_CONTAINER)
        return t == T_CLIP;

    // XXX Currently ClipContainer only into scene
    if (t == T_CLIP_CONTAINER)
        return type() == T_SCENE;

    // Clips only contain sequences
    if (type() == T_CLIP)
        return t & TG_SEQUENCE;

    // microphone groups only contain microphones
    if (type() == T_MICROPHONE_GROUP)
        return t == T_MICROPHONE;

    // audio-units can contain only audio-units
    if (type() == T_AUDIO_UNIT)
        return t == T_AUDIO_UNIT;

    // audio-units can be part of scene and any object
    if (t == T_AUDIO_UNIT)
        return type() == T_SCENE || (type() & TG_REAL_OBJECT);

    // transformations are child-less
    if (type() == T_TRANSFORMATION)
        return false;

    // except for the transformation mix class
    // which holds itself and transformations
    if (type() == T_TRANSFORMATION_MIX)
        return t & TG_TRANSFORMATION;

    // sequences only belong on tracks or sequencegroups with matching type
    // or clips
    if (t & TG_SEQUENCE)
        return
               type() == T_SEQUENCEGROUP
            || type() == T_CLIP
            || (t == T_SEQUENCE_FLOAT && type() == T_TRACK_FLOAT);

    // sequencegroups belong on tracks
    if (t == T_SEQUENCEGROUP)
        return  (type() & TG_TRACK)
                // or themselfes
                || type() == T_SEQUENCEGROUP;

    // tracks contain nothing else but sequences
    if (type() & TG_TRACK)
        return false;

    // sequence contains nothing
    if (type() & TG_SEQUENCE)
        return false;

    // scene can hold anything else, except transformations
    if (type() == T_SCENE)
        return !(t & TG_TRANSFORMATION);

    return true;
}


bool Object1::isSaveToAdd(Object1 * o, QString& error) const
{
    if (!o)
    {
        error = QObject::tr("Object is NULL");
        return false;
    }

    if (!canHaveChildren(o->type()))
    {
        error = QObject::tr("'%1' can not have a child of this type").arg(idName());
        return false;
    }

#if 0
    // test for modulation loops
    QList<Object*> mods = o->getFutureModulatingObjects(sceneObject());

#ifdef MO_DO_DEBUG_MOD
    MO_DEBUG_MOD("--- " << mods.size() << " modulators for " << o->idName());
    for (auto m : mods)
        MO_DEBUG_MOD(m->idName());
#endif

    if (mods.contains((Object*)this))
    {
        error = tr("Adding '%1' as a child to '%2' would cause an infinite "
                   "modulation loop!").arg(o->idName()).arg(idName());
        return false;
    }
#endif

    return true;
}

// ---------------------- tree events ------------------------

void Object1::onObjectsAboutToDelete(const QList<Object1 *> &list)
{
    for (Parameter * p : p_o_->parameters)
    {
        for (const Object1 * o : list)
            if (p->findModulator(o->idName()))
                p->removeModulator(o->idName());
    }
}



// ############################ modulations #############################################

bool Object1::isModulated() const
{
    for (auto p : p_o_->parameters)
        if (p->isModulated())
            return true;

    return false;
}

QList<Object1*> Object1::getModulatingObjects() const
{
    return QList<Object1*>();
}

QList<QPair<Parameter*, Object1*>> Object1::getModulationPairs() const
{
    return QList<QPair<Parameter*, Object1*>>();
}

QList<Object1*> Object1::getFutureModulatingObjects(const Scene * /*scene*/) const
{
    return QList<Object1*>();
}

void Object1::requestCreateOutputs()
{

}


} // namespace MO
