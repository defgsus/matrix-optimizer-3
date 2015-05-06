/** @file parametercallback.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MOSRC_OBJECT_PARA_PARAMETERCALLBACK_H
#define MOSRC_OBJECT_PARA_PARAMETERCALLBACK_H

#include <functional>

#include "parameter.h"

namespace MO {

/** A parameter that exposes a button that can be clicked */
class ParameterCallback : public Parameter
{
public:

    ParameterCallback(Object * object, const QString& idName, const QString& name);
    ~ParameterCallback();

    virtual void serialize(IO::DataStream&) const;
    virtual void deserialize(IO::DataStream&);

    const QString& typeName() const Q_DECL_OVERRIDE { static QString s("callback"); return s; }

    virtual Modulator * getModulator(const QString&, const QString&) Q_DECL_OVERRIDE { return 0; }

    // ---------------- getter -----------------

    std::function<void()> getCallback() const { return p_func_; }

    // ---------------- setter -----------------

    void setCallback(std::function<void()> func) { p_func_ = func; }

    // --------- gui -----------

    /** Execute the callback */
    void fire() { if (p_func_) p_func_(); }

private:

    std::function<void()> p_func_;

};

} // namespace MO

#endif // MOSRC_OBJECT_PARA_PARAMETERCALLBACK_H
