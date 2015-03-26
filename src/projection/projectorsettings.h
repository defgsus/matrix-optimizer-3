/** @file projectorsettings.h

    @brief Settings of a single projector

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/26/2014</p>
*/

#ifndef MOSRC_PROJECTION_PROJECTORSETTINGS_H
#define MOSRC_PROJECTION_PROJECTORSETTINGS_H

#include <QString>
#include <QVector>

#include "types/float.h"
#include "types/vector.h"

namespace MO {
namespace IO { class XmlStream; }

/** Settings of one single projector */
class ProjectorSettings
{
public:

    ProjectorSettings();

    // ------------ io ------------

    void serialize(IO::XmlStream&) const;
    void deserialize(IO::XmlStream&);

    // ---------- getter ----------

    /** Compares the two projector settings */
    bool operator == (const ProjectorSettings&) const;

    /** Unique id (per ProjectionSystemSettings) */
    int id() const { return id_; }
    const QString& name() const { return name_; }
    int width() const { return width_; }
    int height() const { return height_; }
    Float aspect() const { return Float(width())/height(); }
    Float fov() const { return fov_; }
    Float lensRadius() const { return lensRadius_; }
    Float latitude() const { return latitude_; }
    Float longitude() const { return longitude_; }
    Float distance() const { return distance_; }
    Float pitch() const { return pitch_; }
    Float yaw() const { return yaw_; }
    Float roll() const { return roll_; }
    Float offsetX() const { return offsetX_; }
    Float offsetY() const { return offsetY_; }
    Float fisheyeness() const { return fisheyeness_; }

    uint numOverlapAreas() const { return overlapAreas_.count(); }
    const QVector<Vec2> & overlapArea(uint idx) const { return overlapAreas_[idx]; }

    // -------- setter ------------

    void setId(int id) { id_ = id; }
    void setName(const QString& name) { name_ = name; }
    void setWidth(int v) { width_ = v; }
    void setHeight(int v) { height_ = v; }
    void setFov(Float v) { fov_ = v; }
    void setLensRadius(Float v) { lensRadius_ = std::max(Float(0.0001), v); }
    void setLatitude(Float v) { latitude_ = v; }
    void setLongitude(Float v) { longitude_ = v; }
    void setDistance(Float v) { distance_ = v; }
    void setPitch(Float v) { pitch_ = v; }
    void setYaw(Float v) { yaw_ = v; }
    void setRoll(Float v) { roll_ = v; }
    void setOffsetX(Float v) { offsetX_ = v; }
    void setOffsetY(Float v) { offsetY_ = v; }
    void setFisheyeness(Float v) { fisheyeness_ = v; }

    void clearOverlapAreas() { overlapAreas_.clear(); }
    void appendOverlapArea(const QVector<Vec2>& a) { overlapAreas_.append(a); }

    //______________ PRIVATE AREA _________________
private:

    // ---------- config ----------

    QString name_;
    int id_, width_, height_;
    Float
        fov_,
        lensRadius_,
        latitude_,
        longitude_,
        distance_,
        pitch_,
        yaw_,
        roll_,
        offsetX_,
        offsetY_,
        fisheyeness_;

    QVector<QVector<Vec2>> overlapAreas_;
};

} // namespace MO

#endif // MOSRC_PROJECTION_PROJECTORSETTINGS_H
