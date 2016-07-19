/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/15/2016</p>
*/

#ifndef MOSRC_OBJECT_PYTHONOBJECT_H
#define MOSRC_OBJECT_PYTHONOBJECT_H

#include "Object.h"

namespace MO {

class PythonObject : public Object
{
public:
    MO_OBJECT_CONSTRUCTOR(PythonObject);

    virtual Type type() const Q_DECL_OVERRIDE { return T_ANGELSCRIPT; }
    virtual bool isScript() const Q_DECL_OVERRIDE { return true; }

    virtual void createParameters() Q_DECL_OVERRIDE;
    virtual void onParameterChanged(Parameter *p) Q_DECL_OVERRIDE;

signals:

public slots:

    /** Run the script. To be called by GUI thread only! */
    void runScript();

private:

    class Private;
    Private * p_;

};

} // namespace MO

#endif // MOSRC_OBJECT_PYTHONOBJECT_H
