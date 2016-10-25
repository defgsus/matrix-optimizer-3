/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/4/2016</p>
*/

#include "ParameterFloatMatrix.h"
#include "ModulatorFloatMatrix.h"
#include "object/Scene.h"
#include "object/util/ObjectEditor.h"
#include "gui/FloatMatrixDialog.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {

ParameterFloatMatrix::ParameterFloatMatrix(
        Object * object, const QString& id, const QString& name)
    : Parameter (object, id, name)
    , diag_     (nullptr)
{
}


void ParameterFloatMatrix::serialize(IO::DataStream &io) const
{
    Parameter::serialize(io);

    io.writeHeader("parfm", 1);

    io << baseValue_;

}

void ParameterFloatMatrix::deserialize(IO::DataStream &io)
{
    Parameter::deserialize(io);

    io.readHeader("parfm", 1);

    io >> baseValue_;
}

void ParameterFloatMatrix::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterFloatMatrix*>(other);
    if (!p)
        return;
    defaultValue_ = p->defaultValue_;
    baseValue_ = p->baseValue_;
}

QString ParameterFloatMatrix::getDocType() const
{
    QString str = typeName();

    // XXX

    return str;
}

int ParameterFloatMatrix::getModulatorTypes() const
{
    return 0xffffffff;
}

bool ParameterFloatMatrix::hasChanged(const RenderTime& t) const
{
    for (auto m : modulators())
        return static_cast<ModulatorFloatMatrix*>(m)->hasChanged(t);

    if (t.thread() >= hasChanged_.size())
        return true;
    return hasChanged_[t.thread()];
}

FloatMatrix ParameterFloatMatrix::value(const RenderTime& time) const
{
    for (auto m : modulators())
        return static_cast<ModulatorFloatMatrix*>(m)->value(time);

    if (time.thread() >= hasChanged_.size())
        hasChanged_.resize(time.thread()+1);

    hasChanged_[time.thread()] = false;
    return baseValue_;
}

void ParameterFloatMatrix::setValue(const FloatMatrix& v)
{
    baseValue_ = v;
    for (auto i = hasChanged_.begin(); i!=hasChanged_.end(); ++i)
        *i = true;
}


Modulator * ParameterFloatMatrix::getModulator(
        const QString& id, const QString& outputId)
{
    Modulator * m = findModulator(id, outputId);
    if (m)
        return m;

    m = new ModulatorFloatMatrix(idName(), id, outputId, this, object());
    addModulator_(m);

    return m;
}


GUI::FloatMatrixDialog* ParameterFloatMatrix::openEditDialog(QWidget *parent)
{
    MO_ASSERT(object(), "no object for ParameterFloatMatrix::openFileDialog()");
    MO_ASSERT(object()->sceneObject(),
              "no scene for ParameterFloatMatrix::openFileDialog()");
    MO_ASSERT(object()->sceneObject()->editor(),
              "no editor for ParameterFloatMatrix::openFileDialog()");

//    if (!object() || !object()->sceneObject() || !object()->sceneObject()->editor())
//        return 0;

    const QString parName = QString("%1.%2")
            .arg(object()->name()).arg(displayName());

    if (!diag_)
    {
        // prepare default dialog
        diag_ = new GUI::FloatMatrixDialog(parent);
        diag_->setWindowTitle(parName + " " + diag_->windowTitle());
        diag_->setAttribute(Qt::WA_DeleteOnClose, true);
        diag_->setModal(false);

        diag_->setFloatMatrix(baseValue());

        diag_->connect(diag_, &GUI::FloatMatrixDialog::matrixChanged, [this]()
        {
            object()->editor()->setParameterValue(this, diag_->floatMatrix());
        });

        diag_->connect(diag_, &GUI::FloatMatrixDialog::destroyed, [this]()
        {
            diag_ = nullptr;
        });

    }
    else
        diag_->setFloatMatrix(baseValue());

    diag_->show();
    diag_->raise();

    return diag_;
}



} // namespace MO
