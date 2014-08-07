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

    const QString& uniformNameProjection() const { return unProj_; }
    const QString& uniformNameView() const { return unView_; }
    const QString& uniformNameTransformation() const { return unTrans_; }
    const QString& uniformNameLightPos() const { return unLightPos_; }
    const QString& uniformNameLightColor() const { return unLightColor_; }

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

private:

    void addDefine_(QString& src, const QString& def_line) const;

    QString vert_, frag_,
        unProj_, unView_, unTrans_, unLightPos_, unLightColor_,
        anPos_, anCol_, anNorm_, anTexCoord_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SHADERSOURCE_H
