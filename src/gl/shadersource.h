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
    const QString& attribNamePosition() const { return anPos_; }
    const QString& attribNameColor() const { return anCol_; }

    // --------- setter ---------------

    void setVertextSource(const QString& s) { vert_ = s; }
    void setFragmentSource(const QString& s) { frag_ = s; }

    void setAttributeNamePosition(const QString& s) { anPos_ = s; }
    void setAttributeNameColor(const QString& s) { anCol_ = s; }

private:
    QString vert_, frag_,
        unProj_, unView_,
        anPos_, anCol_;
};

} // namespace GL
} // namespace MO

#endif // MOSRC_GL_SHADERSOURCE_H
