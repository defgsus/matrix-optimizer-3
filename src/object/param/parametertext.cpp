/** @file parametertext.cpp

    @brief A text Parameter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/25/2014</p>
*/

#include "parametertext.h"
#include "io/datastream.h"
#include "io/error.h"
#include "io/log.h"
#include "object/trackfloat.h"
#include "object/scene.h"
#include "modulator.h"
#include "io/files.h"


// make ParameterText useable in QMetaObject::invokeMethod
Q_DECLARE_METATYPE(MO::ParameterText*);
namespace { static int register_param = qMetaTypeId<MO::ParameterText*>(); }


namespace MO {

ParameterText::ParameterText(
        Object * object, const QString& id, const QString& name)
    : Parameter     (object, id, name),
      textType_     (TT_PLAIN_TEXT)
{
}


void ParameterText::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("partxt", 1);

    io << value_;
}

void ParameterText::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("partxt", 1);

    io >> value_;
}

void ParameterText::setVariableNames(const std::vector<std::string> &names)
{
    varNames_.clear();
    for (auto & n : names)
        varNames_ << QString::fromStdString(n);
}


} // namespace MO
