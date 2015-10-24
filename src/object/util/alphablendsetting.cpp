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
#include "object/param/parameters.h"
#include "object/param/parameterselect.h"

// Does this even work with qlinguist?
namespace { QString tr(const char * c) { return QObject::tr(c); } }

namespace MO {

const QStringList AlphaBlendSetting::modeIds =
{
    "p",
    "off",
    "mix", "mixb", "mixd",
    "add", "addb", "addd",
    "sub", "subb", "subd"
};

const QStringList AlphaBlendSetting::modeNames =
{
    tr("parent"),
    tr("off"),
    tr("mix"),
    tr("mix light"),
    tr("mix dark"),
    tr("add"),
    tr("add light"),
    tr("add dark"),
    tr("sub"),
    tr("sub light"),
    tr("sub dark")
};


AlphaBlendSetting::AlphaBlendSetting(Object * parent)
    : object_       (parent),
      parentMode_   (M_MIX),
      p_type_       (0)
{
}

void AlphaBlendSetting::createParameters(Mode defaultType, bool with_parent, const QString &suff)
{
    createParameters(defaultType, with_parent, "", suff);
}

void AlphaBlendSetting::createParameters(Mode defaultType, bool with_parent, const QString& prefix, const QString &suff)
{
    MO_ASSERT(object_, "");

    p_type_ = object_->params()->createSelectParameter(prefix + "blendmode" + suff,
                                    tr("alpha blending"),
                                    tr("Selects how succesive drawing actions are composed"),
                                   modeIds,
                                   modeNames,
                                   { tr("Uses the same setting as the parent object"),
                                     tr("No alpha blending occures - the alpha value is ignored"),
                                     tr("The colors are cross-faded depending on the alpha value"),
                                     tr("The lighter colors are cross-faded"),
                                     tr("The darker colors are cross-faded"),
                                     tr("The colors are added together according to alpha value"),
                                     tr("The lighter colors are added together"),
                                     tr("The darker colors are added together"),
                                     tr("The colors are subtracted according to alpha value"),
                                     tr("The lighter colors are subtracted"),
                                     tr("The darker colors are subtracted")
                                   },
                                   { M_PARENT, M_OFF,
                                     M_MIX, M_MIX_BRIGHT, M_MIX_DARK,
                                     M_ADD, M_ADD_BRIGHT, M_ADD_DARK,
                                     M_SUB, M_SUB_BRIGHT, M_SUB_DARK
                                   },
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

bool AlphaBlendSetting::hasAlpha() const
{
    const auto m = mode();
    return m == M_PARENT
            || m == M_ADD
            || m == M_MIX
            || m == M_SUB;
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

void AlphaBlendSetting::apply(const RenderTime& time)
{
    using namespace gl;

    apply( (Mode)p_type_->value(time) );
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

        if (mode == M_SUB || mode == M_SUB_BRIGHT || mode == M_SUB_DARK)
            MO_CHECK_GL( glBlendEquationSeparate(GL_FUNC_REVERSE_SUBTRACT, GL_FUNC_ADD) )
        else
            MO_CHECK_GL( glBlendEquation(GL_FUNC_ADD) );

        switch(mode)
        {
            case M_SUB:
            case M_ADD: MO_CHECK_GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE) ); break;
            case M_SUB_BRIGHT:
            case M_ADD_BRIGHT: MO_CHECK_GL( glBlendFunc(GL_SRC_COLOR, GL_ONE) ); break;
            case M_SUB_DARK:
            case M_ADD_DARK: MO_CHECK_GL( glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE) ); break;

            case M_MIX: MO_CHECK_GL( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) ); break;
            case M_MIX_BRIGHT: MO_CHECK_GL( glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR) ); break;
            case M_MIX_DARK: MO_CHECK_GL( glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_SRC_COLOR) ); break;

            case M_OFF:
            case M_PARENT: break;
        }
    }
}


} // namespace MO
