/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 1/6/2016</p>
*/

#ifndef MSRC_GL_NEUROGL_H
#define MSRC_GL_NEUROGL_H

#include <QSize>

#include "gl/opengl_fwd.h"

namespace MO {

/** A modular neuronal network module using shaders and textures */
class NeuroGl
{
public:

    enum Mode
    {
        MODE_BYPASS = 0,
        MODE_FPROP,
        MODE_BPROP,
        MODE_WEIGHT_INIT
    };

    enum Slot
    {
        SLOT_INPUT = 0,
        SLOT_WEIGHT,
        SLOT_ERROR,
        SLOT_PREV_OUT
    };

    // -------- ctor -----------

    NeuroGl();
    ~NeuroGl();

    // ------- getter ----------

    Mode mode() const;

    float learnrate() const;
    int randomSeed() const;
    float weightInitAmp() const;
    float weightInitOffset() const;
    float weightInitLocalAmp() const;
    float weightInitLocalPow() const;

    const QSize& inputRes() const;
    const QSize& outputRes() const;
    const QSize& weightRes() const;

    int outputFormat() const;
    /** Dimension of cell state [1,4] (float or vecX) */
    int typeDimension() const;

    bool isInputSigned() const;
    bool isOutputSigned() const;
    /** Should alpha channel always be clamped to 1.0 */
    bool isClampAlpha() const;

    QString infoString() const;

    // ------ setter ------------

    void setMode(Mode m);

    void setLearnrate(float lr);
    void setRandomSeed(int s);
    void setWeightInitAmp(float);
    void setWeightInitOffset(float);
    void setWeightInitLocalAmp(float);
    void setWeightInitLocalPow(float);

    void setInputRes(const QSize& si);
    void setOutputRes(const QSize& si);
    void setWeightRes(const QSize& si);

    void setInputTexture(const GL::Texture*);
    void setWeightTexture(const GL::Texture*);
    void setErrorTexture(const GL::Texture*);

    void setOutputFormat(int glenum);
    void setTypeDimension(int);

    void setInputSigned(bool);
    void setOutputSigned(bool);
    void setClampAlpha(bool);

    // ------ opengl ------------

    void releaseGl();

    /** Get processed output, or NULL */
    const GL::Texture* outputTexture() const;
    /** Get processed weights, or NULL */
    const GL::Texture* weightTexture() const;

    /** Lazily initialize all resources that have changed */
    void updateGl();

    void step(int iterations);

private:
    struct Private;
    Private * p_;
};

} // namespace MO

#endif // NEUROGL_H
