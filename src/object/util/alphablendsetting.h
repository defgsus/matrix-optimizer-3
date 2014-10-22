/** @file alphablendsetting.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 22.10.2014</p>
*/

#ifndef MOSRC_OBJECT_UTIL_ALPHABLENDSETTING_H
#define MOSRC_OBJECT_UTIL_ALPHABLENDSETTING_H

#include <QString>

#include "object/object_fwd.h"
#include "types/float.h"

namespace MO {

class AlphaBlendSetting
{
public:
    AlphaBlendSetting(Object *parent = 0);

    /** Type of alpha mix */
    enum Mode
    {
        M_PARENT,
        M_OFF,
        M_MIX,
        M_ADD,
        M_MIX_BRIGHT
    };

    static const QStringList modeNames;
    static const QStringList modeIds;

    // ---------- parameters -----------

    /** Creates the blending-related parameters in parent Object.
        Each parameter id is appended with @p id_suffix, to enable
        more than one option set for an Object. */
    void createParameters(Mode defaultMode, bool with_parent, const QString& id_suffix);

    /** Sets the visibility of the parameters according to current settings. */
    void updateParameterVisibility();

    bool parametersCreated() const { return p_type_; }

    /** Returns true when Parameter @p p is one of this setting class */
    bool hasParameter(Parameter * p) const;

    /** Sets the mode that should be used in M_PARENT mode
        M_PARENT itself is illegal for this function. */
    void setParentMode(Mode mode);

    // ---------- opengl ---------------

    Mode mode() const;

    Mode parentMode() const { return parentMode_; }

    /** Sets the opengl state according to current parameter values */
    void apply(Double time, uint thread);

    /** Sets the opengl state, ignoring current parameters. */
    void apply(Mode mode);

    /** Disables alpha blending in opengl state */
    void disable();

private:

    Object * object_;

    Mode parentMode_;

    ParameterSelect * p_type_;
};

} // namespace MO

#endif // MOSRC_OBJECT_UTIL_ALPHABLENDSETTING_H
