/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/


#include "python34widget.h"
#include "python/34/python.h"
#include "tool/syntaxhighlighter.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {


class Python34Widget::Private
{
public:
    Private(Python34Widget * widget)
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

    Python34Widget * widget;

    SyntaxHighlighter * syn;
};



Python34Widget::Python34Widget(QWidget *parent)
    : AbstractScriptWidget  (IO::FT_TEXT_PYTHON34, parent),
      p_                    (new Private(this))
{
    p_->createObjects();
}

Python34Widget::~Python34Widget()
{
    delete p_;
}

void Python34Widget::Private::createObjects()
{
    if (!syn)
        syn = new SyntaxHighlighter(true, widget);
#ifdef MO_ENABLE_PYTHON34
    syn->initForPython(NULL);
#endif
    widget->setSyntaxHighlighter(syn);
}

QString Python34Widget::getHelpUrl(const QString& token) const
{
    QString url = "python34.html#" + token.toHtmlEscaped();
    return url;
}

bool Python34Widget::compile()
{
    return p_->compile();
}

void Python34Widget::executeScript()
{
    p_->execute();
}

bool Python34Widget::Private::compile()
{
    return true;
}

void Python34Widget::Private::execute()
{
#ifdef MO_ENABLE_PYTHON34
    PYTHON34::PythonInterpreter py;

    try
    {
        py.execute(widget->scriptText());
        widget->setErrorFrom(&py);
    }
    catch (const Exception& e)
    {
        widget->setErrorFrom(&py);
        widget->addCompileMessage(0, M_ERROR, e.what());
    }
#endif
}

void Python34Widget::setErrorFrom(const PYTHON34::PythonInterpreter* inter)
{
    Q_UNUSED(inter);
#ifdef MO_ENABLE_PYTHON34
    QString outp = inter->errorOutput();
    if (outp.isEmpty())
        return;

    /* Find the line number in the python message.
       XXX Currently this only captures the first error,
       don't know if there are more possible */
    int ln = 0;
    int idx = outp.indexOf(", line");
    if (idx > 0)
    {
        idx += 7;
        int idx2 = idx;
        while (idx2 < outp.size() && outp[idx2].isDigit())
            ++idx2;
        if (idx2 > idx)
            ln = outp.mid(idx, idx2-idx).toInt();
    }

    addCompileMessage(ln, M_ERROR, outp);
#endif
}

} // namespace GUI
} // namespace MO


