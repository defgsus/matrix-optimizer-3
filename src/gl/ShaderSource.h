/** @file shadersource.h

    @brief Container for GLSL source code

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#ifndef MOSRC_GL_SHADERSOURCE_H
#define MOSRC_GL_SHADERSOURCE_H

#include <functional>

#include <QString>

namespace MO {
namespace GL {

/** A glorified string container for glsl snippets */
class ShaderSource
{
public:
    ShaderSource();

    // -------- helper --------

    /** Returns first line in @p src which contains @p text, or -1 */
    static int findLineNumber(const QString& src, const QString& text);
    static void replaceWithLineNumber(QString& src, const QString& before, const QString& after);

    /** Adds the line number to the beginning of each line */
    static QString addLineNumbers(const QString& s);

    // ---------- getter --------------

    /** Returns true when all of the sources are empty. */
    bool isEmpty() const { return vert_.isEmpty() && frag_.isEmpty(); }

    const QString& vertexSource() const { return vert_; }
    const QString& fragmentSource() const { return frag_; }
    const QString& geometrySource() const { return geom_; }

    bool operator == (const ShaderSource& rhs) const;
    bool operator != (const ShaderSource& rhs) const { return !(*this == rhs); }

    // standard uniform names

    const QString& uniformNameSceneTime() const { return unSceneTime_; }
    const QString& uniformNameProjection() const { return unProj_; }
    const QString& uniformNameCubeViewTransformation() const { return unCVT_; }
    const QString& uniformNameViewTransformation() const { return unVT_; }
    const QString& uniformNameTransformation() const { return unT_; }

    const QString& uniformNameColor() const { return unColor_; }
    const QString& uniformNameLightAmt() const { return unLightAmt_; }
    const QString& uniformNameBumpScale() const { return unBumpScale_; }

    const QString& uniformNameLightPos() const { return unLightPos_; }
    const QString& uniformNameLightColor() const { return unLightColor_; }
    const QString& uniformNameLightDirection() const { return unLightDir_; }
    const QString& uniformNameLightDirectionParam() const { return unLightDirPar_; }
    const QString& uniformNameLightDiffuseExponent() const { return unLightDiffExp_; }

    const QString& attribNamePosition() const { return anPos_; }
    const QString& attribNameColor() const { return anCol_; }
    const QString& attribNameNormal() const { return anNorm_; }
    const QString& attribNameTexCoord() const { return anTexCoord_; }

    // --------- setter ---------------

    void setVertextSource(const QString& s) { vert_ = s; }
    void setFragmentSource(const QString& s) { frag_ = s; }
    void setGeometrySource(const QString& s) { geom_ = s; }

    void setAttributeNamePosition(const QString& s) { anPos_ = s; }
    void setAttributeNameColor(const QString& s) { anCol_ = s; }

    // ------------- files ------------

    void loadVertexSource(const QString& filename);
    void loadFragmentSource(const QString& filename);
    void loadGeometrySource(const QString& filename);

    /** loads the default shader for, e.g., a Model3d */
    void loadDefaultSource();

    // ---------- manipulation --------

    /** Adds a define to all sources.
        Defines are added after all #version, #extension and other #define lines.
        An #undef can also be added that way. */
    void addDefine(const QString& defineCommand, bool addAfterOtherDefines = true);

    /** Replaces a piece of text */
    void replace(const QString& before, const QString& after, bool adjustLineNumber = false);

    /** Replaces all #include ".." or <..> statements with the result from the function @p func.
        @p func is given the url from the include statement. If it returns an empty string,
        an #error message is inserted instead.
        A true boolean parameter in @p func signals the <> syntax.
        Each #include will only be considered once and recursive includes
        are resolved properly. */
    void replaceIncludes(std::function<QString(const QString&, bool)> func);

    /** Replacess all #include <...> statements for which a file is in the app resources */
    void pasteDefaultIncludes();

    /** To be called before compilation.
        Adds default defines. */
    void finalize();

private:

    void p_addDefine_(QString& src, const QString& def_line, bool addBefore = false) const;
    void p_pasteIncludes_(QString& src, std::function<QString(const QString&, bool)> func, int lvl);

    QString vert_, frag_, geom_,
        unSceneTime_,
        unProj_, unCVT_, unVT_, unT_,
        unLightAmt_, unBumpScale_,
        unLightPos_, unLightColor_, unLightDir_, unLightDirPar_,
        unLightDiffExp_,
        unColor_,
        anPos_, anCol_, anNorm_, anTexCoord_;

    bool finalized_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SHADERSOURCE_H
