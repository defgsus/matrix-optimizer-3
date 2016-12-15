"""
_CPP_(HEADER):
    namespace MO { namespace MATH { class TimelineNd; template <typename T> class ArithmeticArray; } }
_CPP_:
    #include "math/TimelineNd.h"
    #include "py_mo_helper.h"

    using namespace PyUtils;

    #define MO__ASSERT_TL(struct__) \
    if (struct__->tl == nullptr) \
    { \
        setPythonError(PyExc_ReferenceError, "attached timeline is NULL"); \
        return NULL; \
    } \
    if (struct__->tl->numDimensions() == 0) \
    { \
        setPythonError(PyExc_ReferenceError, \
                "timeline dimensionality is NULL"); \
        return NULL; \
    }
"""

from impl.timeline import Timeline