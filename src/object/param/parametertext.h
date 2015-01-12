/** @file parametertext.h

    @brief A text Parameter

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/25/2014</p>
*/

#ifndef MOSRC_OBJECT_PARAM_PARAMETERTEXT_H
#define MOSRC_OBJECT_PARAM_PARAMETERTEXT_H

#include <QStringList>

#include "parameter.h"
#include "object/object_fwd.h"

namespace MO {


class ParameterText : public Parameter
{
public:

    ParameterText(Object * object, const QString& idName, const QString& name);

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("text"); return s; }

    Modulator * getModulator(const QString&, const QString&) Q_DECL_OVERRIDE { return 0; }

    // ---------------- getter -----------------

    QString value() const { return value_; }
    const QString& defaultValue() const { return defaultValue_; }
    const QString& baseValue() const { return value_; }
    TextType textType() const { return textType_; }

    /** For TT_EQUATION this returns a list of variables the equation parser should know */
    const QStringList& variableNames() const { return varNames_; }

    // ---------------- setter -----------------

    void setValue(const QString& fn) { value_ = fn; }
    void setDefaultValue(const QString& fn) { defaultValue_ = fn; }
    void setTextType(TextType t) { textType_ = t; }

    void setVariableNames(const QStringList& names) { varNames_ = names; }
    void setVariableNames(const std::vector<std::string>& names);

    void setVariableDescriptions(const QStringList& descs) { varDescs_ = descs; }
    void setVariableDescriptions(const std::vector<std::string>& descs);

    /** Opens a dialog to edit the text.
        Depending on the textType(), the dialog will be structured.
        If a (valid) text change was done, the scene object will be called with
        Scene::setParameterValue().
        This call immediately returns after opening the dialog.
        XXX Right now, it's modal - otherwise the parameter must take down all it's dialogs on destruction!
        @note The scene MUST be present in the object tree for this call! */
    void openEditDialog(QWidget * parent = 0);

private:

    QString value_, defaultValue_;
    TextType textType_;
    QStringList varNames_, varDescs_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERTEXT_H
