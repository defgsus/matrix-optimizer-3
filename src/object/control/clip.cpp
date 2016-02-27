/** @file clip.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.10.2014</p>
*/

#include <QVariant>

#include "clip.h"
#include "io/datastream.h"
#include "io/error.h"
#include "sequence.h"
#include "clipcontroller.h"
#include "object/param/parameters.h"
#include "object/util/objectmodulatorgraph.h"

#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(Clip)

Clip::Clip()
    : Object          (),
      p_clipContainer_(0),
      p_timeStarted_  (0),
      p_running_      (false)
{
    setName("Clip");
}

void Clip::serialize(IO::DataStream &io) const
{
    Object::serialize(io);

    io.writeHeader("clip", 3);

#ifdef MO__PRE_V3
    io << p_column_ << p_row_;

    // v2
    io << p_color_;
#endif
}

void Clip::deserialize(IO::DataStream &io)
{
    Object::deserialize(io);

    const int ver = io.readHeader("clip", 3);

    if (ver < 3)
    {
        uint col, row;
        io >> col >> row;
        setAttachedData(col, DT_CLIP_COLUMN);
        setAttachedData(row, DT_CLIP_ROW);
        if (ver >= 2)
        {
            QColor co;
            io >> co;
            setAttachedData(co.hslHue(), DT_HUE);
        }
    }
}

void Clip::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("clip", tr("clip"));
    initParameterGroupExpanded("clip");

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

uint Clip::column() const { return getAttachedData(DT_CLIP_COLUMN).toUInt(); }
uint Clip::row() const { return getAttachedData(DT_CLIP_ROW).toUInt(); }
void Clip::setRow(uint row) { setAttachedData(row, DT_CLIP_ROW); }
void Clip::setColumn(uint col) { setAttachedData(col, DT_CLIP_COLUMN); }


QList<Clip*> Clip::getAssociatedClips(Object *o)
{
    QSet<Clip*> clips;

    auto mod = o->getModulatingObjectsList(true);
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
    QList<Object*> mods;

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
        mods << p->getModulatingObjectsList(true);

    // get all objects around and inclusive parameter
    else
    {
        ObjectGraph modg;
        get_object_modulator_graph(modg, obj);

        mods << modg.toList();
    }

    QSet<Clip*> clips;

    // and keep the clips, the modulators are in
    for (auto m : mods)
        if (Object * clip = m->findParentObject(T_CLIP))
            clips.insert(static_cast<Clip*>(clip));

    return clips.toList();
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
