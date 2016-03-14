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
namespace GUI { class TextEditWidget; class TextEditDialog; }

class ParameterText : public Parameter
{
public:

    ParameterText(Object * object, const QString& idName, const QString& name);
    ~ParameterText();

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const { static QString s("text"); return s; }
    SignalType signalType() const Q_DECL_OVERRIDE { return ST_TEXT; }

    Modulator * getModulator(const QString&, const QString&) Q_DECL_OVERRIDE { return 0; }

    QString baseValueString(bool inShort) const override
        { auto s = baseValue(); if (inShort && s.size()>25) { s.resize(23); s += ".."; }
          return s; }
    QString valueString(const RenderTime& , bool inShort) const override
        { return baseValueString(inShort); }

    // ---------------- getter -----------------

    QString value() const { return value_; }
    const QString& defaultValue() const { return defaultValue_; }
    const QString& baseValue() const { return value_; }
    TextType textType() const { return textType_; }

    /** Should be editable in single line or in text editor */
    bool isOneliner() const;

    /** For TT_EQUATION this returns a list of variables the equation parser should know */
    const QStringList& variableNames() const { return varNames_; }

    // ---------------- setter -----------------

    void setValue(const QString& text) { value_ = text; }
    void setDefaultValue(const QString& text) { defaultValue_ = text; }
    void setTextType(TextType t) { textType_ = t; }

    void setVariableNames(const QStringList& names) { varNames_ = names; }
    void setVariableNames(const std::vector<std::string>& names);

    void setVariableDescriptions(const QStringList& descs) { varDescs_ = descs; }
    void setVariableDescriptions(const std::vector<std::string>& descs);

    /** Implemented for scripts.
        Posts messages into the script edit window when open. */
    void addErrorMessage(int line, const QString & text);

    /** Opens a dialog to edit the text.
        Depending on the textType(), the dialog will be structured.
        If a (valid) text change was done, the ObjectEditor will be
        called with ObjectEditor::setParameterValue().
        This call immediately returns after opening the dialog.
        The parameter takes down the dialog on Parameter's destruction!
        @note The scene MUST be present in the object tree for this call!
        @note Don't let parent be the ParameterWidget as it would destroy the
        Dialog when selecting another object. */
    GUI::TextEditDialog * openEditDialog(QWidget * parent = 0);

    /** Creates a widget for editing the text.
        Depending on the textType(), the dialog will be structured.
        If a (valid) text change was done, the ObjectEditor will be
        called with ObjectEditor::setParameterValue().
        The returned widget will issue a closeRequest() signal when the parameter disappears.
        @note The scene MUST be present in the object tree for this call!
        @note Don't let parent be the ParameterWidget as it would destroy the
        Dialog when selecting another object. */
    GUI::TextEditWidget * createEditWidget(QWidget * parent = 0);

    /** Adds an object that is referenced by a source text, if any.
        @p idName is the Object::idName() of a ValueTextInterface. */
    void addIncludeObject(const QString& idName);
    void clearIncludeObjects();

private:

    QString value_, defaultValue_;
    TextType textType_;
    QStringList varNames_, varDescs_;
    QSet<QString> includeIds_;

    GUI::TextEditDialog * diag_;
    GUI::TextEditWidget * editor_;
};

} // namespace MO


#endif // MOSRC_OBJECT_PARAM_PARAMETERTEXT_H
