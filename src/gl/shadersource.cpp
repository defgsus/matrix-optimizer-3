/** @file shadersource.cpp

    @brief Container for GLSL source code

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#include <QFile>
#include <QSet>

#include "shadersource.h"
#include "io/log.h"
#include "io/error.h"

namespace MO {
namespace GL {

ShaderSource::ShaderSource()
    : unSceneTime_  ("u_time"),
      unProj_       ("u_projection"),
      unCVT_        ("u_cubeViewTransform"),
      unVT_         ("u_viewTransform"),
      unT_          ("u_transform"),
      unLightAmt_   ("u_light_amt"),
      unBumpScale_  ("u_bump_scale"),
      unLightPos_   ("u_light_pos[0]"),
      unLightColor_ ("u_light_color[0]"),
      unLightDir_   ("u_light_direction[0]"),
      unLightDirPar_("u_light_direction_param[0]"),
      unLightDiffExp_("u_light_diffuse_exp[0]"),
      unColor_      ("u_color"),
      anPos_        ("a_position"),
      anCol_        ("a_color"),
      anNorm_       ("a_normal"),
      anTexCoord_   ("a_texCoord"),
      finalized_    (false)
{
}

bool ShaderSource::operator == (const ShaderSource& rhs) const
{
    return vert_ == rhs.vert_ && frag_ == rhs.frag_;
}

int ShaderSource::findLineNumber(const QString& src, const QString& text)
{
    int idx = src.indexOf(text);
    if (idx < 0)
        return -1;

    int ln = 0;
    for (int i=idx; i>=0; --i)
        if (src.at(i) == '\n')
            ++ln;
    return ln;
}

void ShaderSource::loadFragmentSource(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "Could not load fragment source from '" << filename << "'\n"
                    << f.errorString());

    frag_ = f.readAll();
    finalized_ = false;
}


void ShaderSource::loadVertexSource(const QString &filename)
{
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly))
        MO_IO_ERROR(READ, "Could not load vertex source from '" << filename << "'\n"
                    << f.errorString());

    vert_ = f.readAll();
    finalized_ = false;
}

void ShaderSource::loadDefaultSource()
{
#if 0
    loadVertexSource(":/shader/test.vert");
    loadFragmentSource(":/shader/test.frag");
#else
    loadVertexSource(":/shader/default.vert");
    loadFragmentSource(":/shader/default.frag");
#endif
}

void ShaderSource::addDefine(const QString &defineCommand, bool addAfter)
{
    p_addDefine_(vert_, defineCommand, !addAfter);
    p_addDefine_(frag_, defineCommand, !addAfter);
}

void ShaderSource::finalize()
{
    if (finalized_)
        return;

    p_addDefine_(vert_, "#define MO_VERTEX", true);
    p_addDefine_(frag_, "#define MO_FRAGMENT", true);
    //MO_PRINT(frag_);
    finalized_ = true;
}

void ShaderSource::replace(const QString &before, const QString &after, bool adjustLineNumber)
{
    if (!adjustLineNumber)
    {
        vert_.replace(before, after);
        frag_.replace(before, after);
    }
    else
    {
        replaceWithLineNumber(vert_, before, after);
        replaceWithLineNumber(frag_, before, after);
    }
}

void ShaderSource::replaceIncludes(std::function<QString (const QString &, bool)> func)
{
    p_pasteIncludes_(vert_, func, 0);
    p_pasteIncludes_(frag_, func, 0);

    //MO_PRINT("[" + frag_ + "]");

    finalized_ = false; // XXX Not exactly sure
}

void ShaderSource::pasteDefaultIncludes()
{
    replaceIncludes([](const QString& url, bool do_search)
    {
        if (!do_search)
            return QString();

        QFile f(":/shader/inc/" + url);
        if (!f.open(QFile::Text | QFile::ReadOnly))
            return QString();

        return "// ----- include '" + url + "' -----\n"
                + QString::fromUtf8(f.readAll());
    });
}

void ShaderSource::p_pasteIncludes_(QString& src, std::function<QString (const QString &, bool)> func, int lvl)
{
    QSet<QString> urls;
    QString cpy;

    int idx = src.indexOf("#include");
    if (idx < 0)
        return;

    bool was_error = false;
    int cidx = 0;
    while (idx >= 0)
    {
        // copy source until here
        cpy += src.mid(cidx, idx - cidx);

        // find end of line
        cidx = idx;
        idx = src.indexOf('\n', cidx+8);
        if (idx < 0)
        {
            cpy += "#error end of line expected\n";
            was_error = true;
            break;
        }

        // get url
        QString url = src.mid(cidx+8, idx - (cidx+8)).simplified();

        if (!urls.contains(url))
        {
            urls.insert(url);

            bool do_search = (url.startsWith("<") && url.endsWith(">"));
            if (!do_search && !(url.startsWith("\"") && url.endsWith("\"")))
            {
                cpy += "#error expected <> or \"\" around name\n";
                was_error = true;
                break;
            }
                else url = url.mid(1, url.length()-2);

            QString inc = func(url, do_search);
            if (inc.isEmpty())
            {
                cpy += "#error not found '" + url + "'\n";
                was_error = true;
                break;
            }

            //int line = findLineNumber()

            // paste
            cpy += "// ----- include '" + url + "' -----\n" + inc;
        }

        // next instance
        cidx = idx;
        idx = src.indexOf("#include", idx);
    }

    // early out
    if (was_error)
    {
        src.swap(cpy);
        return;
    }

    // copy source until here
    cpy += src.mid(cidx);

    src.swap(cpy);

    // resolve recursive includes
    if (lvl < 100 && src.indexOf("#include") >= 0)
        p_pasteIncludes_(src, func, lvl+1);
}


void ShaderSource::replaceWithLineNumber(QString &src, const QString &before, const QString &after)
{
    int idx, offs = 0;
    while (true)
    {
        // find first/next occurence
        idx = src.indexOf(before, offs);
        if (idx < 0)
            break;

        // get line number
        int ln = findLineNumber(src, before);

        // construct replacement string
        QString repl = after + QString("\n#line %1\n").arg(ln + 1);

        // replace string
        src.remove(idx, before.length());
        src.insert(idx, repl);

        offs = idx + repl.length();
    }

//    MO_DEBUG("[" + src + "]");
}

void ShaderSource::p_addDefine_(QString &src, const QString &def_line, bool addBefore) const
{
    QString line = def_line;
    if (!line.endsWith("\n"))
        line.append("\n");

    // add after other defines
    int i = src.lastIndexOf("#define");
    if (!addBefore && i>=0)
    {
        src.insert(src.indexOf("\n", i) + 1, line);
        return;
    }

    // insert after extensions
    i = src.lastIndexOf("#extension");
    if (i>=0)
    {
        src.insert(src.indexOf("\n", i) + 1, line);
        return;
    }

    // insert after version
    i = src.lastIndexOf("#version");
    if (i>=0)
    {
        src.insert(src.indexOf("\n", i) + 1, line);
        return;
    }

    // insert at beginning
    src.prepend(line);
}


} // namespace GL
} // namespace MO
