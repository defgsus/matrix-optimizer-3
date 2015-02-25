/** @file glslwidget.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.01.2015</p>
*/

#include "glslwidget.h"

#include "angelscriptwidget.h"
#include "tool/syntaxhighlighter.h"

namespace MO {
namespace GUI {

GlslWidget::GlslWidget(QWidget * parent)
    : AbstractScriptWidget(parent)
{
    auto syn = new SyntaxHighlighter(this);
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
