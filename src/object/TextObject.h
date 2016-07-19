/** @file textobject.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.05.2015</p>
*/

#ifndef MOSRC_OBJECT_TEXTOBJECT_H
#define MOSRC_OBJECT_TEXTOBJECT_H

#include "Object.h"
#include "object/interface/ValueTextInterface.h"

namespace MO {

class TextObject
        : public Object
        , ValueTextInterface
{
public:
    MO_OBJECT_CONSTRUCTOR(TextObject);

    virtual Type type() const Q_DECL_OVERRIDE { return T_TEXT; }
    virtual bool isText() const Q_DECL_OVERRIDE { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *) Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual QPair<QString, TextType> valueText(uint channel, const RenderTime& time) const
                        Q_DECL_OVERRIDE;
    virtual void valueTextAddErrorMessage(int line, const QString& msg) Q_DECL_OVERRIDE;

    /** Sets the text content and type */
    void setText(const QString& text, TextType type);

private:

    void updateParamType_();

    ParameterText * pText_;
    ParameterSelect * pType_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TEXTOBJECT_H
