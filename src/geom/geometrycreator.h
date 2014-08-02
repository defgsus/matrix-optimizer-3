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

namespace MO {
namespace GEOM {


class GeometryCreator : public QThread
{
    Q_OBJECT
public:
    explicit GeometryCreator(QObject *parent = 0);
    ~GeometryCreator();

    /** After finished() was emitted, the Geometry
        can be taken here. */
    GEOM::Geometry * takeGeometry();

signals:

    /** There was an error create the Geometry */
    void failed(const QString&);

public slots:

    void setSettings(const GeometryFactorySettings&);

protected:

    void run() Q_DECL_OVERRIDE;

    Geometry * geometry_;
    GeometryFactorySettings * settings_;
    ObjLoader * loader_;
};


} // namespace GEOM
} // namespace MO

#endif // GEOMETRYCREATOR_H
