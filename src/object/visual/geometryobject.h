/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/29/2015</p>
*/

#ifndef MOSRC_OBJECT_VISUAL_GEOMETRYOBJECT_H
#define MOSRC_OBJECT_VISUAL_GEOMETRYOBJECT_H

#include "object/object.h"

namespace MO {
namespace GEOM
{
    class GeometryCreator;
    class Geometry;
    class GeometryFactorySettings;
}

/** A simple container for Geometry */
class GeometryObject : public Object
{
    Q_OBJECT
public:

    MO_OBJECT_CONSTRUCTOR(GeometryObject);
    ~GeometryObject();

    Type type() const Q_DECL_OVERRIDE { return T_GEOMETRY; }
    virtual bool isGeometry() const Q_DECL_OVERRIDE { return true; }

    /** Returns the current geometry settings. */
    const GEOM::GeometryFactorySettings& geometrySettings() const;

    /** Sets new geometry settings and creates the geometry on next render */
    void setGeometrySettings(const GEOM::GeometryFactorySettings&);

    /** Overwrite current geometry */
    void setGeometry(const GEOM::Geometry& );

    /** Returns the created Geometry, if any. */
    const GEOM::Geometry * geometry() const;

protected:

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParametersLoaded() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;
    virtual void updateParameterVisibility() Q_DECL_OVERRIDE;

    virtual void getNeededFiles(IO::FileList &files) Q_DECL_OVERRIDE;

signals:

    /** Emitted when the Geometry has been changed or removed.
        Call geometry() to receive it. */
    void geometryChanged();

private slots:

    void createGeometry_();

    void geometryCreated_();
    void geometryFailed_(const QString & e);

    /** Discards the current thread, if any, and sets creator_=0. */
    void resetCreator_();

private:

    GEOM::GeometryCreator * creator_;
    GEOM::GeometryFactorySettings * geomSettings_;
    GEOM::Geometry * geometry_;
};

} // namespace MO


#endif // MOSRC_OBJECT_VISUAL_GEOMETRYOBJECT_H
