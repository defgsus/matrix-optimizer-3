/** @file parametertimeline1d.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.10.2014</p>
*/

#include "parametertimeline1d.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/scene.h"
#include "math/timeline1d.h"
#ifndef MO_CLIENT
#   include "gui/timelineeditdialog.h"
#   include "gui/timeline1dview.h"
#endif

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
    delete default_;
    delete tl_;
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
            tl_ = new MATH::Timeline1D;
        tl_->deserialize(io);
    }
}

MATH::Timeline1D * ParameterTimeline1D::timeline()
{
    // create one?
    if (!tl_)
    {
        tl_ = new MATH::Timeline1D();

        // with default content?
        if (default_)
            *tl_ = *default_;
    }
    return tl_;
}

void ParameterTimeline1D::setTimeline(MATH::Timeline1D *tl)
{
    delete tl_;
    tl_ = tl;
}

void ParameterTimeline1D::setTimeline(const MATH::Timeline1D & tl)
{
    *(timeline()) = tl;
}

void ParameterTimeline1D::setDefaultTimeline(const MATH::Timeline1D & tl)
{
    if (!default_)
        default_ = new MATH::Timeline1D();

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



#ifndef MO_CLIENT
bool ParameterTimeline1D::openEditDialog(QWidget *parent)
{
    MO_ASSERT(object(), "no object for ParameterTimeline1D::openFileDialog()");
    MO_ASSERT(object()->sceneObject(), "no scene for ParameterTimeline1D::openFileDialog()");

    if (!object() || !object()->sceneObject())
        return false;

    const QString parName = QString("%1.%2").arg(object()->name()).arg(name());

    MATH::Timeline1D backup(*timeline());

    // prepare dialog
    GUI::TimelineEditDialog diag(parent);
    diag.setWindowTitle(QObject::tr("timeline %1").arg(parName));
    diag.setTimeline(*timeline());

    if (minTime_ > -infinity)
    {
        GUI::UTIL::ViewSpace space = diag.editor().viewSpace();
        space.setMinX(minTime_);
        diag.editor().setViewSpace(space);
    }
    if (maxTime_ < infinity)
    {
        GUI::UTIL::ViewSpace space = diag.editor().viewSpace();
        space.setMaxX(maxTime_);
        diag.editor().setViewSpace(space);
    }


    bool changed = false;

    diag.connect(&diag, &GUI::TimelineEditDialog::timelineChanged, [this, &diag, &changed]()
    {
        object()->sceneObject()->setParameterValue(this, diag.timeline());
        changed = true;
    });

    // reset to default
    if (diag.exec() == QDialog::Rejected && changed)
        object()->sceneObject()->setParameterValue(this, backup);

    return changed;
}
#endif


} // namespace MO
