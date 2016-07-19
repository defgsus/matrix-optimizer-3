/** @file parametertext.cpp

    @brief A text Parameter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/25/2014</p>
*/

#include "ParameterText.h"
#include "Modulator.h"
#include "object/Scene.h"
#include "object/util/ObjectEditor.h"
#include "object/interface/ValueTextInterface.h"
#include "gui/TextEditDialog.h"
#include "gui/widget/TextEditWidget.h"
#include "io/Files.h"
#include "io/DataStream.h"
#include "io/error.h"
#include "io/log.h"

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
    {
        diag_->close();
        /*
        if (diag_->parent())
            diag_->deleteLater();
        else
            delete diag_;
            */
    }
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

void ParameterText::copyFrom(Parameter* other)
{
    Parameter::copyFrom(other);
    auto p = dynamic_cast<ParameterText*>(other);
    if (!p)
        return;
    defaultValue_ = p->defaultValue_;
    value_ = p->value_;
}

bool ParameterText::isOneliner() const
{
    return textType_ == TT_PLAIN_TEXT
        || textType_ == TT_OBJECT_WILDCARD;
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

    const QString parName = QString("%1.%2")
            .arg(object()->name()).arg(displayName());

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
            object()->editor()->setParameterValue(this, diag_->getText());
        });

        diag_->connect(diag_, &GUI::TextEditDialog::destroyed, [this]()
        {
            diag_ = 0;
        });

    }
    else
        diag_->setText(value_);

    diag_->show();
    diag_->raise();

    return diag_;
}

GUI::TextEditWidget * ParameterText::createEditWidget(QWidget *parent)
{
    MO_ASSERT(object(), "no object for ParameterText::openEditWidget()");
    MO_ASSERT(object()->sceneObject(), "no scene for ParameterText::openEditWidget()");
    MO_ASSERT(object()->sceneObject()->editor(),
              "no editor for ParameterText::openEditWidget()");

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
            object()->editor()->setParameterValue(this, editor_->getText());
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

    // -- follow includes --
    /** @todo CAN RECURSE INDEFINITELY */
    if (object())
    if (auto s = object()->sceneObject())
    {
        for (auto id : includeIds_)
        {
            if (auto o = s->findChildObject(id, true))
            if (auto tf = dynamic_cast<ValueTextInterface*>(o))
            {
                tf->valueTextAddErrorMessage(line, text);
            }
        }
    }
}

void ParameterText::addIncludeObject(const QString &idName)
{
    includeIds_.insert(idName);
}

void ParameterText::clearIncludeObjects()
{
    includeIds_.clear();
}


} // namespace MO
