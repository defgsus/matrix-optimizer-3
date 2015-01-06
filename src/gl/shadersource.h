/** @file shadersource.h

    @brief Container for GLSL source code

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/27/2014</p>
*/

#ifndef MOSRC_GL_SHADERSOURCE_H
#define MOSRC_GL_SHADERSOURCE_H

#include <QString>

namespace MO {
namespace GL {

class ShaderSource
{
public:
    ShaderSource();

    // ---------- getter --------------

    /** Returns true when all of the sources are empty. */
    bool isEmpty() const { return vert_.isEmpty() && frag_.isEmpty(); }

    const QString& vertexSource() const { return vert_; }
    const QString& fragmentSource() const { return frag_; }

    const QString& uniformNameSceneTime() const { return unSceneTime_; }
    const QString& uniformNameProjection() const { return unProj_; }
    const QString& uniformNameCubeViewTransformation() const { return unCVT_; }
    const QString& uniformNameViewTransformation() const { return unVT_; }
    const QString& uniformNameTransformation() const { return unT_; }

    const QString& uniformNameColor() const { return unColor_; }
    const QString& uniformNameDiffuseExponent() const { return unDiffuseExp_; }
    const QString& uniformNameBumpScale() const { return unBumpScale_; }

    const QString& uniformNameLightPos() const { return unLightPos_; }
    const QString& uniformNameLightColor() const { return unLightColor_; }
    const QString& uniformNameLightDirection() const { return unLightDir_; }
    const QString& uniformNameLightDirectionMix() const { return unLightDirMix_; }
    const QString& uniformNameLightDiffuseExponent() const { return unLightDiffExp_; }

    const QString& attribNamePosition() const { return anPos_; }
    const QString& attribNameColor() const { return anCol_; }
    const QString& attribNameNormal() const { return anNorm_; }
    const QString& attribNameTexCoord() const { return anTexCoord_; }

    // --------- setter ---------------

    void setVertextSource(const QString& s) { vert_ = s; }
    void setFragmentSource(const QString& s) { frag_ = s; }

    void setAttributeNamePosition(const QString& s) { anPos_ = s; }
    void setAttributeNameColor(const QString& s) { anCol_ = s; }

    // ------------- files ------------

    void loadVertexSource(const QString& filename);
    void loadFragmentSource(const QString& filename);

    void loadDefaultSource();

    // ---------- manipulation --------

    /** Adds a define to all sources.
        Defines are added after all #version, #extension and other #define lines.
        An #undef can also be added that way. */
    void addDefine(const QString& defineCommand);

    /** Replaces a piece of text */
    void replace(const QString& before, const QString& after);

private:

    void addDefine_(QString& src, const QString& def_line) const;

    QString vert_, frag_,
        unSceneTime_,
        unProj_, unCVT_, unVT_, unT_,
        unDiffuseExp_, unBumpScale_,
        unLightPos_, unLightColor_, unLightDir_, unLightDirMix_,
        unLightDiffExp_,
        unColor_,
        anPos_, anCol_, anNorm_, anTexCoord_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SHADERSOURCE_H
