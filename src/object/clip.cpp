/** @file clip.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include "clip.h"
#include "io/datastream.h"
#include "io/error.h"
#include "sequence.h"
#include "clipcontainer.h"
#include "param/parameters.h"
#include "util/objectmodulatorgraph.h"

#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(Clip)

Clip::Clip(QObject *parent)
    : Object        (parent),
      p_clipContainer_(0),
      p_timeStarted_  (0),
      p_running_      (false),
      p_column_       (0),
      p_row_          (0),
      p_color_        (QColor(50,100,50))
{
    setName("Clip");
}

void Clip::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("clip", 2);

    io << p_column_ << p_row_;

    // v2
    io << p_color_;
}

void Clip::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    const int ver = io.readHeader("clip", 2);

    io >> p_column_ >> p_row_;

    if (ver >= 2)
        io >> p_color_;
}

void Clip::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("clip", tr("clip"));

        paramSpeed_ = params()->createFloatParameter("speed", tr("speed"),
                                        tr("The speed multiplier for all sequences in the clip"),
                                        1, 0.1, true, false);

    params()->endParameterGroup();
}


void Clip::updateParameterVisibility()
{
    Object::updateParameterVisibility();
}


void Clip::childrenChanged()
{
    // get all sequences and sub-sequences
    p_sequences_ = findChildObjects<Sequence>(QString(), true);
}


QList<Clip*> Clip::getAssociatedClips(Object *o)
{
    QSet<Clip*> clips;

    auto mod = o->getModulatingObjects();
    for (auto m : mod)
        if (m->parentObject() && m->parentObject()->isClip())
            clips.insert(static_cast<Clip*>(m->parentObject()));

    QList<Clip*> ret;
    for (auto c : clips)
        ret << c;
    return ret;
}

QList<Clip*> Clip::getAssociatedClips(Parameter * p, int parentMask)
{    
    // list of all modulating objects to the parameter
    QList<Object*> mod;

    // check parent object?
    Object * obj = 0;
    if (parentMask)
    {
        obj = p->object();
        while (obj)
        {
            // no match - no check
            if (!(obj->type() & parentMask))
                obj = 0;
            else
            {
                // go up in tree if parent matches mask
                if (obj->parentObject()
                    && (obj->parentObject()->type() & parentMask))
                    obj = obj->parentObject();
                else
                    break;
            }
        }
    }

    // get all objects modulating this parameter
    if (!obj)
        mod << p->getModulatingObjects();

    // get clips of all objects around and inclusive parameter
    else
    {
        ObjectGraph mods;
        getObjectModulatorGraph(mods, obj);

        mod << mods.toList();
    }


    QSet<Clip*> clips;

    // and keep the clips, the modulators are in
    for (auto m : mod)
        if (Object * clip = m->findParentObject(T_CLIP))
            clips.insert(static_cast<Clip*>(clip));

    QList<Clip*> ret;
    for (auto c : clips)
        ret << c;
    return ret;
}


void Clip::startClip(Double gtime)
{
    p_timeStarted_ = gtime;
    p_running_ = true;
}

void Clip::stopClip()
{
    p_running_ = false;
}


} // namespace MO
