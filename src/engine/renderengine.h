/** @file renderengine.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.01.2015</p>
*/

#ifndef MOSRC_ENGINE_RENDERENGINE_H
#define MOSRC_ENGINE_RENDERENGINE_H

#include <QObject>

#include "types/float.h"

namespace MO {
namespace GL { class Context; }

class Scene;

class RenderEngine : public QObject
{
    Q_OBJECT
public:
    explicit RenderEngine(QObject *parent = 0);
    ~RenderEngine();

    // -------------- getter -------------

    /** Assigned scene */
    Scene * scene() const;

    /** Assigned thread index */
    uint thread() const;

signals:

public slots:

    /** Creates the gl path for the given scene.
        @p thread is the thread-index for which to query Parameters in the scene. */
    void setScene(Scene *, GL::Context * , uint thread);

    /** Initializes gl resources if necessary */
    void render(Double time);

    /** Releases all gl resources if necessary.
        Must be called by thread owning gl-context. */
    void release();

private:

    class Private;
    Private * p_;

};


} // namespace MO

#endif // MOSRC_ENGINE_RENDERENGINE_H
