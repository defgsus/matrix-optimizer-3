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
    explicit Camera(QObject *parent = 0);

    MO_OBJECT_CLONE(Camera)

    virtual const QString& className() const { static QString s(MO_OBJECTCLASSNAME_CAMERA); return s; }

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    virtual Type type() const { return T_CAMERA; }
    virtual bool isCamera() const { return true; }

    virtual void initGl(int thread);
    virtual void renderGl(int thread, Double time);

    virtual void setNumberThreads(int num);

    /** Returns projection matrix */
    const Mat4& projection(int thread) const { return projection_[thread]; }

    /** Starts rendering an openGL frame */
    void startGlFrame(int thread, Double time);

    // XXX only trying here
    void setProjectionMatrix(int thread);
signals:

public slots:

private:

    std::vector<Mat4> projection_;
};

} // namespace MO

#endif // MOSRC_OBJECT_CAMERA_H
