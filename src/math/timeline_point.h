/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/15/2016</p>
*/

#ifndef MOSRC_MATH_TIMELINE_POINT_H
#define MOSRC_MATH_TIMELINE_POINT_H

#include <QString>

namespace MO {
namespace MATH {

/** possible types of points / interpolations */
struct TimelinePoint
{
    enum Type
    {
        /** default is used only to signal to use the last type. <br>
            it's no legal type to calculate with. */
        DEFAULT,
        /** the value stays unchanged over time until
            the next cue-point */
        CONSTANT,
        /** Linear user-defined derivative after each cue-point */
        CONSTANT_USER,
        /** the value will linearly fade between this and the next point */
        LINEAR,
        /** the value will nicely fade between this and the next point
            in a sigmoid fashion */
        SMOOTH,
        /** the value will be interpolated between calculated
            symmetric derivatives at each point. <br>
            curve continuity at each point is granted. */
        SYMMETRIC,
        /** the value will be interpolated between user-adjustable
            symmetric derivatives at each point. <br>
            curve continuity at each point is granted. */
        SYMMETRIC_USER,
        /** the value will be interpolated between user-adjustable
            symmetric derivatives at each point. <br>
            variation of hermite interpolation. */
        SYMMETRIC2,
        /** 4-point spline.
            curve continuity at each point is granted. */
        SPLINE4_SYM,
        /** the value will describe the way of a nice spline (4 points are used) */
        SPLINE4,
        /** the value will describe the way of a very nice spline (6 points are used) */
        SPLINE6,
        /** this is no legal type, only the number of possible values */
        MAX
    };

    /** returns the user-friendly name of the type. */
    static const char *getName(Type type);

    /** returns the <b>persistent</b> name of the type.
        these names <b>must never change</b>!! */
    static const char *getPersistentName(Type type);

    /** Returns the type for the given persistent name,
        or LINEAR if unknown. */
    static Type getTypeForPersistentName(const QString& persistent_name);

    /** Is this type continuous at the actual cue-point */
    static bool isContinuous(Type);
    /** Has the type user adjustable derivatives */
    static bool isUserDerivative(Type);
    /** Has the derivatives that need to be calculated first */
    static bool isAutoDerivative(Type);
    /** Uses the type user adjustable or automatic derivatives */
    static bool hasDerivative(Type t)
        { return isUserDerivative(t) || isAutoDerivative(t); }

};

} // namespace MATH
} // namespace MO

#endif // MOSRC_MATH_TIMELINE_POINT_H

