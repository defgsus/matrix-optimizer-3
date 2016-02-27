/** @file translation.h

    @brief object translation class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef MOSRC_OBJECT_TRANSFORM_TRANSLATION_H
#define MOSRC_OBJECT_TRANSFORM_TRANSLATION_H

#include "transformation.h"

namespace MO {


class Translation : public Transformation
{
public:
    MO_OBJECT_CONSTRUCTOR(Translation);

    virtual void createParameters();

    virtual void applyTransformation(Mat4& matrix, const RenderTime& time) const
                                                                        Q_DECL_OVERRIDE;
protected:

    ParameterFloat * x_, * y_, * z_;
};

} // namespace MO

#endif // MOSRC_OBJECT_TRANSFORM_TRANSLATION_H
