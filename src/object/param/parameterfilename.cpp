/** @file parameterfilename.cpp

    @brief Parameter for filenames

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/13/2014</p>
*/

#include "parameterfilename.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/trackfloat.h"
#include "object/scene.h"
#include "modulator.h"
#include "io/files.h"


// make ParameterFilename useable in QMetaObject::invokeMethod
Q_DECLARE_METATYPE(MO::ParameterFilename*);
namespace { static int register_param = qMetaTypeId<MO::ParameterFilename*>(); }


namespace MO {

ParameterFilename::ParameterFilename(
        Object * object, const QString& id, const QString& name)
    : Parameter     (object, id, name),
      fileType_     (IO::FT_ANY)
{
}


void ParameterFilename::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("parfn", 1);

    io << value_;
}

void ParameterFilename::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("parfn", 1);

    io >> value_;
}


bool ParameterFilename::openFileDialog(QWidget * parent)
{
    MO_ASSERT(object(), "no object for ParameterFilename::openFileDialog()");
    MO_ASSERT(object()->sceneObject(), "no scene for ParameterFilename::openFileDialog()");

    if (!object() || !object()->sceneObject())
        return false;

    QString fn = IO::Files::getOpenFileName(fileType_, parent);
    if (fn.isEmpty())
        return false;

    object()->sceneObject()->setParameterValue(this, fn);
    return true;
}


} // namespace MO
