/** @file parameterfilename.cpp

    @brief Parameter for filenames

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/13/2014</p>
*/

#include "ParameterFilename.h"
#include "Modulator.h"
#include "object/control/TrackFloat.h"
#include "object/Scene.h"
#include "object/util/ObjectEditor.h"
#include "audio/tool/LadspaPlugin.h"
#include "gui/AudioPluginDialog.h"
#include "io/Files.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"



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

void ParameterFilename::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterFilename*>(other);
    if (!p)
        return;
    defaultValue_ = p->defaultValue_;
    value_ = p->value_;
}


QString ParameterFilename::getDocType() const
{
    return IO::fileTypeNames.at(fileType_) + " " + QObject::tr("filename");
}

bool ParameterFilename::openFileDialog(QWidget * parent)
{
    MO_ASSERT(object(), "no object for ParameterFilename::openFileDialog()");
    MO_ASSERT(object()->sceneObject(), "no scene for ParameterFilename::openFileDialog()");
    MO_ASSERT(object()->sceneObject()->editor(), "no Editor for ParameterFilename::openFileDialog()");

    if (!object() || !object()->sceneObject())
        return false;

    QString fn;
    switch (fileType_)
    {
        default:
            fn = IO::Files::getOpenFileName(fileType_, parent);
        break;

#ifndef MO_DISABLE_LADSPA
        case IO::FT_LADSPA:
        {
            auto plug = GUI::AudioPluginDialog::selectPlugin(parent);
            if (plug)
            {
                fn = plug->idName();
                plug->releaseRef("ParameterFilename::openDialog getplugname done");
            }
        }
        break;
#endif
    }

    if (fn.isEmpty())
        return false;

    object()->sceneObject()->editor()->setParameterValue(this, fn);
    return true;
}


} // namespace MO
