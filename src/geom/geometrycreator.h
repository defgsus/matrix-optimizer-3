/** @file geometrycreator.h

    @brief Threaded GeometryFactory::createFromSettings invoker

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_GUI_UTIL_GEOMETRYCREATOR_H
#define MOSRC_GUI_UTIL_GEOMETRYCREATOR_H

#include <QThread>

#include "gl/opengl_fwd.h"

class QTimer;
class QMutex;

namespace MO {
namespace GEOM {


/** Thread to create a geometry from a GeometryFactorySettings class */
class GeometryCreator : public QThread
{
    Q_OBJECT
public:
    explicit GeometryCreator(QObject *parent = 0);
    ~GeometryCreator();

    /** After succeeded() was emitted, the Geometry
        can be taken here. */
    GEOM::Geometry * takeGeometry();

signals:

    /** Returns progress of processing in percent */
    void progress(int);

    /** There was an error creating the Geometry */
    void failed(const QString&);
    /** All okay, use takeGeometry() now */
    void succeeded();

public slots:

    void setSettings(const GeometryFactorySettings&);

protected slots:

    void onTimer_();

protected:

    void run() Q_DECL_OVERRIDE;

    static int instanceCounter_;

    Geometry * geometry_, * curGeometry_;
    GeometryFactorySettings * settings_;
    QTimer * timer_;
    /** Mutex around settings_ and geometry_ */
    QMutex * mutex_;
};


} // namespace GEOM
} // namespace MO

#endif // GEOMETRYCREATOR_H
