/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/28/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#ifndef MOSRC_PYTHON_34_PYTHON_H
#define MOSRC_PYTHON_34_PYTHON_H

#include <QString>

namespace MO {
class Object;
namespace GEOM { class Geometry; }
namespace PYTHON34 {

    /** Python 3.4 interpreter.
        This class wraps an interpreter instance
        and associates state to it.
        @todo Not tested with multiple instances yet
    */
    class PythonInterpreter
    {
    public:
        PythonInterpreter();
        ~PythonInterpreter();

        static QString getHelpHtmlString();

        void execute(const char* utf8);
        void execute(const QString&);

        /* Deletes the current interpreter */
        //void clear();

        /** The current runtime output */
        const QString& output() const;
        /** The current runtime error output */
        const QString& errorOutput() const;

        // ----------- state ------------

        /** Returns the current Interpreter that runs
            the PyThreadState, or NULL */
        static PythonInterpreter* current();
        static long instanceCount();

        void setGeometry(GEOM::Geometry*);
        GEOM::Geometry* getGeometry() const;

        void setObject(Object*);
        Object* getObject() const;

        // ------- output callback ------

        void write(const char* utf8, bool error = false);

    private:
        struct Private;
        Private * p_;
    };


    /** Called automatically, forget about this */
    void initPython();

    /** Call this at the end of application.
        Does nothing if initPython() has not been called. */
    void finalizePython();


    void runConsole(int argc, char** args);

} // namespace PYTHON34
} // namespace MO

#endif // MOSRC_PYTHON_34_PYTHON_H

#endif // #ifdef MO_ENABLE_PYTHON34
