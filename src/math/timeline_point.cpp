/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/15/2016</p>
*/

#include "timeline_point.h"

namespace MO {
namespace MATH {

const char *TimelinePoint::getName(Type type)
{
    switch (type)
    {
        default:
        case DEFAULT:           return "default";               break;
        case CONSTANT:          return "constant";              break;
        case CONSTANT_USER:     return "constant-derivative";   break;
        case LINEAR:            return "linear";                break;
        case SMOOTH:            return "smooth*";               break;
        case SYMMETRIC:         return "symmetric*";            break;
        case SYMMETRIC_USER:    return "symmetric-derivative*"; break;
        case SYMMETRIC2:        return "hermite";               break;
        case SPLINE4_SYM:       return "symmetric4*";           break;
        case SPLINE4:           return "spline4";               break;
        case SPLINE6:           return "spline6";               break;
    }
}

const char *TimelinePoint::getPersistentName(Type type)
{
    switch (type)
    {
        default:
        case DEFAULT:           return "def";           break;
        case CONSTANT:          return "const";         break;
        case CONSTANT_USER:     return "const_der";     break;
        case LINEAR:            return "linear";        break;
        case SMOOTH:            return "smooth";        break;
        case SYMMETRIC:         return "sym";           break;
        case SYMMETRIC_USER:    return "sym_der";       break;
        case SYMMETRIC2:        return "hermite";       break;
        case SPLINE4_SYM:       return "sym4";          break;
        case SPLINE4:           return "spline4";       break;
        case SPLINE6:           return "spline6";       break;
    }
}

TimelinePoint::Type TimelinePoint::getTypeForPersistentName(const QString& name)
{
    for (int i=DEFAULT; i<MAX; ++i)
        if (name == getPersistentName((Type)i))
            return (Type)i;
    return LINEAR;
}


bool TimelinePoint::isUserDerivative(Type type)
{
    return type == TimelinePoint::CONSTANT_USER
        || type == TimelinePoint::SYMMETRIC_USER;
}

bool TimelinePoint::isAutoDerivative(Type type)
{
    return (type == TimelinePoint::SYMMETRIC ||
            type == TimelinePoint::SYMMETRIC2);

}

bool TimelinePoint::isContinuous(Type type)
{
    return (type == TimelinePoint::SMOOTH ||
            type == TimelinePoint::SYMMETRIC ||
            type == TimelinePoint::SYMMETRIC_USER ||
            type == TimelinePoint::SPLINE4_SYM
            );
}



} // namespace MATH
} // namespace MO


