/** @file model3d.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/29/2014</p>
*/

#ifndef MODEL3D_H
#define MODEL3D_H


#include "objectgl.h"

namespace MO {

class Model3d : public ObjectGl
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Model3d);

    virtual void initGl(uint thread);
    virtual void renderGl(uint, Double time);

signals:

public slots:

};

} // namespace MO

#endif // MODEL3D_H
