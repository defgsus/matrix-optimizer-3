/** @file float.cpp

    @brief float type and utility

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#include "float.h"

Q_DECLARE_METATYPE(MO::Float);
Q_DECLARE_METATYPE(MO::Double);

namespace
{
    static int register_float = qRegisterMetaType<MO::Float>("Float");
    static int register_float2 = qRegisterMetaType<MO::Float>("MO::Float");
    static int register_double = qRegisterMetaType<MO::Double>("Double");
    static int register_double2 = qRegisterMetaType<MO::Double>("MO::Double");
}

