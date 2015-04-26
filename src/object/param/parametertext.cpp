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
#include "object/control/trackfloat.h"
#include "object/scene.h"
#include "object/util/objecteditor.h"
#include "modulator.h"
#include "io/files.h"
#include "gui/texteditdialog.h"
#include "gui/widget/texteditwidget.h"

// make ParameterText useable in QMetaObject::invokeMethod
Q_DECLARE_METATYPE(MO::ParameterText*);
namespace { static int register_param = qMetaTypeId<MO::ParameterText*>(); }


namespace MO {

ParameterText::ParameterText(
        Object * object, const QString& id, const QString& name)
    : Parameter     (object, id, name),
      textType_     (TT_PLAIN_TEXT),
      diag_         (0),
      editor_       (0)
{
}

ParameterText::~ParameterText()
{
    if (diag_)
        diag_->deleteLater();
    if (editor_)
        emit editor_->closeRequest();
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

GUI::TextEditDialog* ParameterText::openEditDialog(QWidget *parent)
{
    MO_ASSERT(object(), "no object for ParameterText::openFileDialog()");
    MO_ASSERT(object()->sceneObject(), "no scene for ParameterText::openFileDialog()");
    MO_ASSERT(object()->sceneObject()->editor(), "no editor for ParameterText::openFileDialog()");

//    if (!object() || !object()->sceneObject() || !object()->sceneObject()->editor())
//        return 0;

    const QString parName = QString("%1.%2").arg(object()->name()).arg(name());

    if (!diag_)
    {
        // prepare default dialog
        diag_ = new GUI::TextEditDialog(value_, textType_, parent);
        diag_->setAttribute(Qt::WA_DeleteOnClose, true);
        diag_->setModal(false);

        // copy equation namespace
        if (textType_ == TT_EQUATION)
        {
            diag_->addVariableNames(varNames_, varDescs_);
            diag_->setWindowTitle(QObject::tr("equation for %1").arg(parName));
        }

        diag_->connect(diag_, &GUI::TextEditDialog::textChanged, [this]()
        {
            object()->sceneObject()->editor()->setParameterValue(this, diag_->getText());
        });

        diag_->connect(diag_, &GUI::TextEditDialog::destroyed, [this]()
        {
            diag_ = 0;
        });

    }
    else
        diag_->setText(value_);

    diag_->show();

    return diag_;
}

GUI::TextEditWidget * ParameterText::createEditWidget(QWidget *parent)
{
    MO_ASSERT(object(), "no object for ParameterText::openFileDialog()");
    MO_ASSERT(object()->sceneObject(), "no scene for ParameterText::openFileDialog()");
    MO_ASSERT(object()->sceneObject()->editor(), "no editor for ParameterText::openFileDialog()");

//    if (!object() || !object()->sceneObject() || !object()->sceneObject()->editor())
//        return 0;

    const QString parName = QString("%1.%2").arg(object()->name()).arg(name());

    if (!editor_)
    {
        // prepare default dialog
        editor_ = new GUI::TextEditWidget(value_, textType_, parent);
        editor_->setAttribute(Qt::WA_DeleteOnClose, true);
        editor_->setObjectName("_TextEditWidget_" + idName());

        // copy equation namespace
        if (textType_ == TT_EQUATION)
        {
            editor_->addVariableNames(varNames_, varDescs_);
            editor_->setWindowTitle(QObject::tr("equation for %1").arg(parName));
        }

        editor_->connect(editor_, &GUI::TextEditWidget::textChanged, [this]()
        {
            object()->sceneObject()->editor()->setParameterValue(this, editor_->getText());
        });

        editor_->connect(editor_, &GUI::TextEditWidget::destroyed, [this]()
        {
            editor_ = 0;
        });

    }

    return editor_;
}

void ParameterText::addErrorMessage(int line, const QString &text)
{
    if (editor_)
        editor_->addErrorMessage(line, text);
    if (diag_)
        diag_->addErrorMessage(line, text);
}

} // namespace MO
