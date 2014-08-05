/** @file modulator.h

    @brief Abstract modulator class

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/6/2014</p>
*/

#ifndef MOSRC_ENGINE_MODULATOR_H
#define MOSRC_ENGINE_MODULATOR_H

namespace MO {

class Object;

class Modulator
{
public:

    /** Construct a modulator as part of an object */
    Modulator(Object * obj = 0);

    // ------------- getter --------------

    Object * object() const { return object_; }

private:

    Object * object_;
};

} // namespace MO

#endif // MOSRC_ENGINE_MODULATOR_H
