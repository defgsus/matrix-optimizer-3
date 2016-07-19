/** @file parametertimeline1d.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.10.2014</p>
*/

#include "ParameterTimeline1d.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/Scene.h"
#include "object/util/ObjectEditor.h"
#include "math/Timeline1d.h"
#include "gui/TimelineEditDialog.h"
#include "gui/Timeline1dView.h"

// make ParameterTimeline1D useable in QMetaObject::invokeMethod
Q_DECLARE_METATYPE(MO::ParameterTimeline1D*);
namespace { static int register_param = qMetaTypeId<MO::ParameterTimeline1D*>(); }

namespace MO {

Double ParameterTimeline1D::infinity = 1e100;

ParameterTimeline1D::ParameterTimeline1D(Object * object, const QString& id, const QString& name)
    :   Parameter       (object, id, name),
        tl_             (0),
        default_        (0),
        minValue_       (-infinity),
        maxValue_       (+infinity),
        minTime_        (-infinity),
        maxTime_        (+infinity)
{
}

ParameterTimeline1D::~ParameterTimeline1D()
{
    if (default_)
        default_->releaseRef("ParameterTimeline1D destroy");
    if (tl_)
        tl_->releaseRef("ParameterTimeline1D destroy");
}

void ParameterTimeline1D::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("partl", 1);

    // timeline
    io << (quint8)(tl_ != 0);
    if (tl_)
        tl_->serialize(io);

}

void ParameterTimeline1D::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("partl", 1);

    quint8 have;
    io >> have;
    if (have)
    {
        if (!tl_)
            tl_ = new MATH::Timeline1d;
        tl_->deserialize(io);
    }
}

void ParameterTimeline1D::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterTimeline1D*>(other);
    if (!p)
        return;
    if (p->default_)
    {
        if (!default_)
            default_ = new MATH::Timeline1d;
        *default_ = *p->default_;
    }
    if (p->tl_)
    {
        if (!tl_)
            tl_ = new MATH::Timeline1d;
        *tl_ = *p->tl_;
    }
    minValue_ = p->minValue_;
    maxValue_ = p->maxValue_;
    minTime_ = p->minTime_;
    maxTime_ = p->maxTime_;
}


MATH::Timeline1d * ParameterTimeline1D::timeline()
{
    // create one?
    if (!tl_)
    {
        tl_ = new MATH::Timeline1d();

        // with default content?
        if (default_)
            *tl_ = *default_;
    }
    return tl_;
}

const MATH::Timeline1d & ParameterTimeline1D::getDefaultTimeline()
{
    if (!default_)
        default_ = new MATH::Timeline1d();

    return *default_;
}

void ParameterTimeline1D::setTimeline(MATH::Timeline1d *tl)
{
    if (tl_)
        tl_->releaseRef("ParameterTimeline1D setnew relprev");
    tl_ = tl;
}

void ParameterTimeline1D::setValue(const MATH::Timeline1d & tl)
{
    *(timeline()) = tl;
}

void ParameterTimeline1D::setDefaultTimeline(const MATH::Timeline1d & tl)
{
    if (!default_)
        default_ = new MATH::Timeline1d();

    *default_ = tl;
}

void ParameterTimeline1D::reset()
{
    if (!tl_)
        return;

    // delete
    setTimeline(0);
    // and lazy recreate..
}



bool ParameterTimeline1D::openEditDialog(QWidget *parent)
{
    MO_ASSERT(object(), "no object for ParameterTimeline1D::openFileDialog()");
    MO_ASSERT(object()->sceneObject(), "no scene for ParameterTimeline1D::openFileDialog()");
    MO_ASSERT(object()->sceneObject()->editor(), "no editor for ParameterTimeline1D::openFileDialog()");

    if (!object() || !object()->sceneObject())
        return false;

    const QString parName = QString("%1.%2").arg(object()->name()).arg(name());

    auto backup = new MATH::Timeline1d(*timeline());
    ScopedRefCounted tldel(backup, "ParameterTimeline1D backup");

    // prepare dialog
    GUI::TimelineEditDialog diag(parent);
    diag.setWindowTitle(QObject::tr("timeline %1").arg(parName));
    diag.setTimeline(*timeline());

    if (minTime_ > -infinity)
    {
        GUI::UTIL::ViewSpace space = diag.editor().viewSpace();
        space.setMinX(minTime_);
        diag.setViewSpace(space);
    }
    if (maxTime_ < infinity)
    {
        GUI::UTIL::ViewSpace space = diag.editor().viewSpace();
        space.setMaxX(maxTime_);
        diag.setViewSpace(space);
    }

    if (minValue_ > -infinity)
    {
        GUI::UTIL::ViewSpace space = diag.editor().viewSpace();
        space.setMinY(minValue_);
        diag.setViewSpace(space);
    }
    if (maxValue_ < infinity)
    {
        GUI::UTIL::ViewSpace space = diag.editor().viewSpace();
        space.setMaxY(maxValue_);
        diag.setViewSpace(space);
    }


    bool changed = false;

    diag.connect(&diag, &GUI::TimelineEditDialog::timelineChanged,
                 [this, &diag, &changed]()
    {
        object()->sceneObject()->editor()->setParameterValue(this, diag.timeline());
        changed = true;
    });

    // reset to default
    if (diag.exec() == QDialog::Rejected && changed)
        object()->sceneObject()->editor()->setParameterValue(this, *backup);

    return changed;
}


} // namespace MO
