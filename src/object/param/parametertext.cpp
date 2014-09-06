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
#ifndef MO_CLIENT
#   include "gui/texteditdialog.h"
#endif

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

void ParameterText::setVariableDescriptions(const std::vector<std::string> &descs)
{
    varDescs_.clear();
    for (auto & n : descs)
        varDescs_ << QString::fromStdString(n);
}

#ifndef MO_CLIENT
bool ParameterText::openEditDialog(QWidget *parent)
{
    MO_ASSERT(object(), "no object for ParameterFilename::openFileDialog()");
    MO_ASSERT(object()->sceneObject(), "no scene for ParameterFilename::openFileDialog()");

    if (!object() || !object()->sceneObject())
        return false;

    QString oldText = value_;

    const QString parName = QString("%1.%2").arg(object()->name()).arg(name());

    // prepare dialog
    GUI::TextEditDialog diag(value_, textType_, parent);
    if (textType_ == TT_EQUATION)
    {
        diag.addVariableNames(varNames_, varDescs_);
        diag.setWindowTitle(QObject::tr("equation for %1").arg(parName));
    }
    else
        diag.setWindowTitle(parName);

    diag.connect(&diag, &GUI::TextEditDialog::textChanged, [this, &diag]()
    {
        object()->sceneObject()->setParameterValue(this, diag.getText());
    });

    diag.exec();

    return oldText != value_;
}
#endif

} // namespace MO
