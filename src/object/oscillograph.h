/** @file oscillograph.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#ifndef MOSRC_OBJECT_OSCILLOGRAPH_H
#define MOSRC_OBJECT_OSCILLOGRAPH_H


#include "objectgl.h"

namespace MO {

class Oscillograph : public ObjectGl
{
    Q_OBJECT
public:
    enum LightMode
    {
        LM_NONE,
        LM_PER_VERTEX,
        LM_PER_FRAGMENT
    };


    MO_OBJECT_CONSTRUCTOR(Oscillograph);

    const GEOM::Geometry * geometry() const;
    Vec4 modelColor(Double time, uint thread) const;

protected:

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings& rs, uint, Double time) Q_DECL_OVERRIDE;
//    virtual void numberLightSourcesChanged(uint thread) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

//    virtual void getNeededFiles(IO::FileList &files) Q_DECL_OVERRIDE;


private:

    void setupDrawable_();
    void calcVaoBuffer_(Double time, uint thread);

    GL::Drawable * draw_;
    GEOM::Geometry * nextGeometry_;

    ParameterFloat * paramR_, *paramG_, *paramB_, *paramA_, *paramBright_,
                    * paramTimeSpan_,
                    * paramValue_,
                    * paramWidth_,
                    * paramLineWidth_;
    ParameterInt * paramNumPoints_;

    bool doRecompile_;

    std::vector<gl::GLfloat> vaoBuffer_;
};

} // namespace MO

#endif // MOSRC_OBJECT_OSCILLOGRAPH_H
