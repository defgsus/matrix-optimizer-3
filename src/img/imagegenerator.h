/** @file imagegenerator.h

    @brief Image creation and editing functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef IMAGEGENERATOR_H
#define IMAGEGENERATOR_H

#include "types/float.h"

namespace MO {

class Image;

class ImageGenerator
{
public:

    /** Creates a normalmap from @p src */
    static void createNormalmap(Image * dst, const Image * src, Float height = 0.1f);

};

} // namespace MO

#endif // IMAGEGENERATOR_H
