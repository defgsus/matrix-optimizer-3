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
namespace GUI {
namespace UTIL {


class GeometryCreator : public QThread
{
    Q_OBJECT
public:
    explicit GeometryCreator(QObject *parent = 0);
    ~GeometryCreator();

    /** After finished() was emitted, the Geometry
        can be taken here. */
    GL::Geometry * takeGeometry();

signals:

    /** There was an error create the Geometry */
    void failed(const QString&);

public slots:

    void setSettings(const GL::GeometryFactorySettings&);

protected:

    void run() Q_DECL_OVERRIDE;

    GL::Geometry * geometry_;
    GL::GeometryFactorySettings * settings_;
};


} // namespace UTIL
} // namespace GUI
} // namespace MO

#endif // GEOMETRYCREATOR_H
