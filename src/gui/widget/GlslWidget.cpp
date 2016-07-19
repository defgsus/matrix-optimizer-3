/** @file glslwidget.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.01.2015</p>
*/

#include "GlslWidget.h"

#include "AngelscriptWidget.h"
#include "tool/SyntaxHighlighter.h"

namespace MO {
namespace GUI {

GlslWidget::GlslWidget(QWidget * parent)
    : AbstractScriptWidget(IO::FT_TEXT_GLSL, parent)
{
    auto syn = new SyntaxHighlighter(false, this);
    syn->initForGlsl();
    setSyntaxHighlighter(syn);
}


bool GlslWidget::compile()
{
    // XXX
    return true;
}

QString GlslWidget::getHelpUrl(const QString& /*token*/) const
{
    return "glsl.html";
}

} // namespace GUI
} // namespace MO
