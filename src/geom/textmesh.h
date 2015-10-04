#ifndef MOSRC_GEOM_TEXTMESH_H
#define MOSRC_GEOM_TEXTMESH_H

#include <QString>
#include <QFont>

namespace MO {
class Properties;
namespace GEOM {

class Geometry;

class TextMesh
{
public:

    enum Mode
    {
        M_POLYLINE,
        M_TESS
    };

    TextMesh();
    ~TextMesh();

    // ---- getter ----

    const Properties& properties() const;

    // --- setter ---

    void setProperties(const Properties&);

    void setText(const QString&);
    void setFont(const QFont&);

    /** Geometry will be added to @p g */
    void getGeometry(Geometry*g) const;

private:
    struct Private;
    Private * p_;
};


} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_TEXTMESH_H
