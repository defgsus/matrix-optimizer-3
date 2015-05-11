/** @file textobject.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 09.05.2015</p>
*/

#include "textobject.h"
#include "object/param/parameters.h"
#include "object/param/parametertext.h"
#include "object/param/parameterselect.h"
#include "object/scene.h"
#include "io/datastream.h"

namespace MO {

MO_REGISTER_OBJECT(TextObject)

TextObject::TextObject(QObject *parent) :
    Object(parent)
{
    setName("Text");
}

void TextObject::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("text", 1);
}

void TextObject::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("text", 1);
}



void TextObject::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("text", tr("text"));
    initParameterGroupExpanded("text");

        pType_ = params()->createSelectParameter(
                "text_type", tr("type"),
                tr("The type of text in this object"),
                { "text", "glsl", "as", "equ" },
                { tr("Text"), tr("GLSL source"), tr("AngelScript source"), tr("Equation") },
                { tr("A context-free text"), tr("Useable as include from other shaders"),
                            tr("*Currently no purpose*"), tr("*Currently no purpose*") },
                { TT_PLAIN_TEXT, TT_GLSL, TT_ANGELSCRIPT, TT_EQUATION },
                TT_PLAIN_TEXT,
                true, false);

        pText_ = params()->createTextParameter("text", tr("text"), tr("The text stored in this object"),
                TT_PLAIN_TEXT, "", true, false);

    params()->endParameterGroup();
}

void TextObject::onParameterChanged(Parameter * p)
{
    Object::onParameterChanged(p);

    if (p == pType_)
        updateParamType_();

    if (p == pText_ && sceneObject())
        sceneObject()->notifyParameterChange(pText_);
}

void TextObject::onParametersLoaded()
{
    Object::onParametersLoaded();
    updateParamType_();
}

void TextObject::updateParameterVisibility()
{
    Object::updateParameterVisibility();

    updateParamType_();
}

void TextObject::updateParamType_()
{
    pText_->setTextType((TextType)pType_->baseValue());
    /*
    switch (pType_->baseValue())
    {
        case TT_PLAIN_TEXT: pText_->setStatusTip();
    }*/
}

QPair<QString, TextType> TextObject::valueText(Double , uint ) const
{
    return qMakePair(pText_->value(), pText_->textType());
}

} // namespace MO
