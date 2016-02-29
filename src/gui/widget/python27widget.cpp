/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/


#ifdef MO_ENABLE_PYTHON27

#include "python27widget.h"
#include "python/2.7/python.h"
#include "tool/syntaxhighlighter.h"
#include "io/error.h"
#include "io/log.h"


namespace MO {
namespace GUI {


class Python27Widget::Private
{
public:
    Private(Python27Widget * widget)
        : widget    (widget),
          syn       (0)
    {

    }

    ~Private()
    {
        //delete engine;
    }

    void createObjects();

    bool compile();

    void execute();

    Python27Widget * widget;

    SyntaxHighlighter * syn;
};



Python27Widget::Python27Widget(QWidget *parent)
    : AbstractScriptWidget  (IO::FT_TEXT_PYTHON27, parent),
      p_                    (new Private(this))
{
    p_->createObjects();
}

Python27Widget::~Python27Widget()
{
    delete p_;
}

void Python27Widget::Private::createObjects()
{
    if (!syn)
        syn = new SyntaxHighlighter(widget);
    //syn->initForPython27();

    widget->setSyntaxHighlighter(syn);
}

QString Python27Widget::getHelpUrl(const QString& token) const
{
    QString url = "python27.html#" + token.toHtmlEscaped();
    return url;
}

bool Python27Widget::compile()
{
    return p_->compile();
}

void Python27Widget::executeScript()
{
    p_->execute();
}

bool Python27Widget::Private::compile()
{
    return true;
}

void Python27Widget::Private::execute()
{
    try
    {
        PYTHON27::PythonInterpreter py;
        py.execute(widget->scriptText());
    }
    catch (const Exception& e)
    {
        widget->addCompileMessage(0, M_ERROR, e.what());
    }
}

} // namespace GUI
} // namespace MO


#endif // MO_ENABLE_PYTHON27

