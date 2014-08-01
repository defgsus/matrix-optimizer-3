/** @file camera.h

    @brief Camera Object

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/28/2014</p>
*/

#ifndef MOSRC_OBJECT_CAMERA_H
#define MOSRC_OBJECT_CAMERA_H

#include "objectgl.h"

namespace MO {

class Camera : public ObjectGl
{
    Q_OBJECT
public:
    MO_OBJECT_CONSTRUCTOR(Camera);

    virtual Type type() const Q_DECL_OVERRIDE { return T_CAMERA; }
    virtual bool isCamera() const Q_DECL_OVERRIDE { return true; }

    virtual void setNumberThreads(uint num) Q_DECL_OVERRIDE;
    virtual void setBufferSize(uint bufferSize, uint thread) Q_DECL_OVERRIDE;

    virtual void initGl(uint thread) Q_DECL_OVERRIDE;
    virtual void renderGl(const GL::CameraSpace&, uint, Double) Q_DECL_OVERRIDE { };

    virtual GL::FrameBufferObject * getFrameBuffer(uint thread) const { return fbo_[thread]; }

    /** Returns projection matrix */
    const Mat4& projection(uint thread, uint sample) const { return projection_[thread][sample]; }

    /** Starts rendering an openGL frame */
    void startGlFrame(uint thread, Double time);

    /** Initialize camera space (with projection)
        XXX only trying here.. */
    void initCameraSpace(GL::CameraSpace& cam, uint thread, uint sample) const;

    // XXX only trying here
    //void setProjectionMatrix(uint thread, uint sample);
signals:

public slots:

private:

    std::vector<std::vector<Mat4>> projection_;
    std::vector<GL::FrameBufferObject*> fbo_;
};

} // namespace MO

#endif // MOSRC_OBJECT_CAMERA_H
