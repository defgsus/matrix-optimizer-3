/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/17/2015</p>
*/

#ifndef MCWSRC_DECODERFRAME
#define MCWSRC_DECODERFRAME

#include <vector>
#include <cstring>
#include <cinttypes>

//#include "config.h"

namespace MO { namespace GL { class Texture; } }

//MO_BEGIN_NAMESPACE


#define MO_PLANE_UNIFY


/** One YUV (or YCbCr) frame with timing information.
    The memory is 16-bytes-aligned */
class DecoderFrame
{
public:
    /** Default constructor creates an invalid frame without memory */
    DecoderFrame()
        : p_width_      (0)
        , p_height_     (0)
        , p_frameNum_   (-1)
        , p_consec_     (false)
        , p_size_       (0)
        , p_planeY_     (0)
#ifdef MO_PLANE_UNIFY
        , p_planeUV_    (0)
#else
        , p_planeU_     (0)
        , p_planeV_     (0)
#endif
        , p_pts_        (0)
    { }

    /** Constructor creating an empty frame.
        Memory is allocated but not initialized. */
    DecoderFrame(int width, int height, int64_t frameNumber, double presentationTime, bool isConsecutive = true)
        : p_data_       (width * height * 3 / 2 + 15)
        , p_width_      (width)
        , p_height_     (height)
        , p_frameNum_   (frameNumber)
        , p_consec_     (isConsecutive)
        , p_size_       (width * height * 3 / 2)
        , p_pts_        (presentationTime)
    {
        // aligned memory
        p_planeY_ = (uint8_t*)( size_t(&p_data_[0] + 15) & ~size_t(15) );
#ifndef MO_PLANE_UNIFY
        p_planeU_ = p_planeY_ + planeYSize();
        p_planeV_ = p_planeU_ + planeUSize();
#else
        p_planeUV_ = p_planeY_ + planeYSize();
#endif
    }

    // -------- getter ----------

    bool isValid() const { return p_width_ > 0 && p_height_ > 0; }
    /** Is this frame in sequence, or the first after seeking? */
    bool isConsecutive() const { return p_consec_; }

    size_t width() const { return p_width_; }
    size_t height() const { return p_height_; }

    size_t planeYSize() const { return p_width_ * p_height_; }
    size_t planeUSize() const { return (p_width_/2) * (p_height_/2); }
    size_t planeVSize() const { return planeUSize(); }
    size_t planeUVSize() const { return planeUSize() + planeVSize(); }

    /** Returns the memory in bytes needed for all three planes */
    size_t memory() const { return p_size_; }

    int64_t frameNumber() const { return p_frameNum_; }
    double presentationTime() const { return p_pts_; }

    const uint8_t * planeY() const { return p_planeY_; }
#ifndef MO_PLANE_UNIFY
    const uint8_t * planeU() const { return p_planeU_; }
    const uint8_t * planeV() const { return p_planeV_; }
#else
    const uint8_t * planeUV() const { return p_planeUV_; }
#endif
    bool operator < (const DecoderFrame& rhs) const
        { return presentationTime() < rhs.presentationTime(); }

    // -------------- setter ---------------

    /** Sets all plane's pixels to zero */
    void clear()
    {
        memset(p_planeY_, 0, p_size_);
    }

    uint8_t * planeY() { return p_planeY_; }
    void fillPlaneY(const uint8_t* srcY) { memcpy(planeY(), srcY, planeYSize()); }
    void fillPlaneY(const uint8_t* srcY, size_t linesize)
    {
        uint8_t* dst = planeY();
        for (size_t i=0; i<height(); ++i, dst += width(), srcY += linesize)
            memcpy(dst, srcY, width());
    }
#ifndef MO_PLANE_UNIFY
    uint8_t * planeU() { return p_planeU_; }
    uint8_t * planeV() { return p_planeV_; }
    void fillPlaneU(const uint8_t* srcU) { memcpy(planeU(), srcU, planeUSize()); }
    void fillPlaneV(const uint8_t* srcV) { memcpy(planeV(), srcV, planeVSize()); }
#else
    uint8_t * planeUV() { return p_planeUV_; }

    /** Interleaves the two separate planes into one */
    void fillPlaneUV(const uint8_t* srcU, const uint8_t* srcV)
    {
        uint8_t *dst = p_planeUV_;
        const size_t num = planeUSize();
        for (size_t i=0; i<num; ++i)
        {
            *dst++ = *srcU++;
            *dst++ = *srcV++;
        }
    }
    void fillPlaneUV(const uint8_t* srcU, const uint8_t* srcV, int lineSizeU, int lineSizeV)
    {
        uint8_t *dst = planeUV();
        const size_t w = width() / 2, h = height() / 2;
        for (size_t j=0; j<h; ++j, srcU += lineSizeU, srcV += lineSizeV)
        for (size_t i=0; i<w; ++i)
        {
            *dst++ = srcU[i];
            *dst++ = srcV[i];
        }
    }
#endif

    bool isConverted() const { return p_converted_.size() == width()*height()*3; }
    const uint8_t* converted() const { return p_converted_.data(); }

    void convertToYUV();

    MO::GL::Texture* createTextureY() const;
    MO::GL::Texture* createTextureYUV();


private:
    void p_upload_(MO::GL::Texture*, const void*) const;

    std::vector<uint8_t> p_data_, p_converted_;
    int p_width_, p_height_;
    int64_t p_frameNum_;
    bool p_consec_;
    size_t p_size_;
    uint8_t *p_planeY_
#ifndef MO_PLANE_UNIFY
        , *p_planeU_, *p_planeV_;
#else
        , *p_planeUV_;
#endif
    double p_pts_;
};


//MO_END_NAMESPACE


#endif // MCWSRC_DECODERFRAME

