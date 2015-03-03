/** @file modulatorobject.h

    @brief An abstract object that represents a modulator

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/10/2014</p>
*/

#ifndef MOSRC_OBJECT_MODULATOROBJECT_H
#define MOSRC_OBJECT_MODULATOROBJECT_H


#include "object.h"

namespace MO {

class ModulatorObject : public Object
{
    Q_OBJECT
public:
    MO_ABSTRACT_OBJECT_CONSTRUCTOR(ModulatorObject);

    bool isModulatorObject() const Q_DECL_OVERRIDE { return true; }

    /** Sets the id of an GUI::AbstractFrontItem for which this
        ModulatorObject should serve as a proxy.
        The object will be set invisible then. */
    void setUiId(const QString& id) { p_uiId_ = id; setVisible(isUiProxy()); }

    /** Returns the id of an GUI::AbstractFrontItem, if this
        ModulatorObject is a proxy for the ui item. */
    const QString& uiId() const { return p_uiId_; }

    /** Returns true when this ModulatorObject is a proxy for
        an GUI::AbstractFrontItem. */
    bool isUiProxy() const { return !p_uiId_.isEmpty(); }

signals:

public slots:

private:

    QString p_uiId_;
};

} // namespace MO

#endif // MOSRC_OBJECT_MODULATOROBJECT_H
