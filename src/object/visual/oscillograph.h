/** @file oscillograph.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 13.12.2014</p>
*/

#ifndef MOSRC_OBJECT_VISUAL_OSCILLOGRAPH_H
#define MOSRC_OBJECT_VISUAL_OSCILLOGRAPH_H


#include "objectgl.h"

namespace MO {

/** An oscillograph render object. */
class Oscillograph : public ObjectGl
{
public:

    MO_OBJECT_CONSTRUCTOR(Oscillograph);
    ~Oscillograph();

    const GEOM::Geometry * geometry() const;
    Vec4 modelColor(const RenderTime & time) const;

protected:

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void releaseGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::RenderSettings& rs, const RenderTime& time) Q_DECL_OVERRIDE;
//    virtual void numberLightSourcesChanged(uint thread) Q_DECL_OVERRIDE;

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;
    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList &files) Q_DECL_OVERRIDE;


private:

    class Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_OBJECT_VISUAL_OSCILLOGRAPH_H
