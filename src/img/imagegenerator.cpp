/** @file imagegenerator.cpp

    @brief Image creation and editing functions

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#include "imagegenerator.h"
#include "image.h"
#include "types/vector.h"

namespace MO {

void ImageGenerator::createNormalmap(Image *dst, const Image *src, Float height)
{
    dst->resize(src->width(), src->height(), Image::F_RGB_24);
    for (uint y=0; y<src->height(); ++y)
    for (uint x=0; x<src->width(); ++x)
    {
        const Vec3 n = glm::normalize(Vec3(
                (Float)(src->average((x+1)%src->width(), y)
                        - src->average((x-1+src->width())%src->width(), y)) / Image::max(),
                (Float)(src->average(x, (y+1)%src->height())
                        - src->average(x, (y-1+src->height())%src->height())) / Image::max(),
                height));

        auto dstp = dst->pixel(x, y);
        *dstp++ = n[0] * Image::max();
        *dstp++ = n[1] * Image::max();
        *dstp   = n[2] * Image::max();
    }
}


} // namespace MO
