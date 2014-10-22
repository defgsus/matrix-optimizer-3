/** @file alphablendsetting.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 22.10.2014</p>
*/

#include <QObject> // for tr()

#include "alphablendsetting.h"
#include "gl/opengl.h"
#include "object/object.h"
#include "object/param/parameterselect.h"

// Does this even work with qlinguist?
namespace { QString tr(const char * c) { return QObject::tr(c); } }

namespace MO {

const QStringList AlphaBlendSetting::modeIds =
{
    "p", "off", "mix", "add", "addb"
};

const QStringList AlphaBlendSetting::modeNames =
{
    tr("parent"),
    tr("off"),
    tr("mix"),
    tr("add"),
    tr("mix brightness")
};


AlphaBlendSetting::AlphaBlendSetting(Object * parent)
    : object_       (parent),
      parentMode_   (M_MIX),
      p_type_       (0)
{
}

void AlphaBlendSetting::createParameters(Mode defaultType, bool with_parent, const QString &suff)
{
    p_type_ = object_->createSelectParameter("_blendmode" + suff,
                                    tr("alpha blending"),
                                    tr("Selects how succesive drawing actions are composed"),
                                   modeIds,
                                   modeNames,
                                   { tr("Uses the same setting as the parent object"),
                                     tr("No alpha blending occures - the alpha value is ignored"),
                                     tr("The colors are cross-faded depending on the alpha value"),
                                     tr("The colors are added together"),
                                     tr("The lighter colors are cross-faded")},
                                   { M_PARENT, M_OFF, M_MIX, M_ADD, M_MIX_BRIGHT },
                                   defaultType, true, false);
    if (!with_parent)
        p_type_->removeByValue(M_PARENT);
}

bool AlphaBlendSetting::hasParameter(Parameter *p) const
{
    return p == p_type_;
}

AlphaBlendSetting::Mode AlphaBlendSetting::mode() const
{
    return p_type_ ? Mode(p_type_->baseValue())
                   : M_MIX;
}

void AlphaBlendSetting::setParentMode(Mode mode)
{
    if (mode == M_PARENT)
    {
        MO_GL_WARNING("AlphaBlendSetting::apply(Mode) with M_PARENT is undefined");
        return;
    }

    parentMode_ = mode;
}

void AlphaBlendSetting::disable()
{
    MO_CHECK_GL( gl::glDisable(gl::GL_BLEND) );
}

void AlphaBlendSetting::apply(Double time, uint thread)
{
    using namespace gl;

    apply( (Mode)p_type_->value(time, thread) );
}

void AlphaBlendSetting::apply(Mode mode)
{
    using namespace gl;

    // change to parent's mode
    if (mode == M_PARENT)
        mode = parentMode_;

    // disable
    if (mode == M_OFF)
        MO_CHECK_GL( glDisable(GL_BLEND) )

    // enable
    else
    {
        MO_CHECK_GL( glEnable(GL_BLEND) );

        switch(mode)
        {
            case M_ADD: MO_CHECK_GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) ); break;
            case M_MIX: MO_CHECK_GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ); break;
            case M_MIX_BRIGHT: MO_CHECK_GL( glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR) ); break;

            case M_OFF:
            case M_PARENT: break;
        }
    }
}


} // namespace MO
