/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON27

#ifndef MOSRC_PYTHON_27_PYTHON_H
#define MOSRC_PYTHON_27_PYTHON_H

#include <QString>

namespace MO {
namespace GEOM { class Geometry; }
namespace PYTHON27 {

    class PythonInterpreter
    {
    public:
        PythonInterpreter();
        ~PythonInterpreter();

        void execute(const char* utf8);
        void execute(const QString&);

        /** Deletes the current interpreter */
        void clear();

        // ----------- state ------------

        void setGeometry(MO::GEOM::Geometry*);
        MO::GEOM::Geometry* geometry() const;

    private:
        struct Private;
        Private * p_;
    };


    /** Called automatically, forget about this */
    void initPython();

    /** Call this at the end of application.
        Does nothing if initPython() has not been called. */
    void finalizePython();


} // namespace PYTHON27
} // namespace MO

#endif // MOSRC_PYTHON_27_PYTHON_H

#endif // #ifdef MO_ENABLE_PYTHON27
