/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/10/2016</p>
*/

#ifndef MOSRC_OBJECT_INTERFACE_GEOMETRYEDITINTERFACE
#define MOSRC_OBJECT_INTERFACE_GEOMETRYEDITINTERFACE

namespace MO {
namespace GUI { class GeometryDialog; }
namespace GEOM { class GeometryFactorySettings; }

class GeometryEditInterface
{
public:

    GeometryEditInterface() : p_gei_diag_(0) { }
    ~GeometryEditInterface();

    GUI::GeometryDialog * getAttachedGeometryDialog() const { return p_gei_diag_; }
    void setAttachedGeometryDialog(GUI::GeometryDialog* d);

    /** Returns the current geometry settings. */
    virtual const GEOM::GeometryFactorySettings& getGeometrySettings() const = 0;

    /** Sets new geometry settings with blocking or lazy creation.. */
    virtual void setGeometrySettings(const GEOM::GeometryFactorySettings&) = 0;

private:

    GUI::GeometryDialog * p_gei_diag_;
};

} // namespace MO

#endif // MOSRC_OBJECT_INTERFACE_GEOMETRYEDITINTERFACE

