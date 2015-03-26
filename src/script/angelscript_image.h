/** @file angelscript_image.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.01.2015</p>
*/

#ifndef MO_DISABLE_ANGELSCRIPT

#ifndef MOSRC_SCRIPT_ANGELSCRIPT_IMAGE_H
#define MOSRC_SCRIPT_ANGELSCRIPT_IMAGE_H

#include <QString>

class asIScriptEngine;
class QImage;

namespace MO {

/** Put the image related types and functions into the namespace.
    Dependency: vector, math */
void registerAngelScript_image(asIScriptEngine *engine);

/** Registers the name 'image' for the instance of QImage */
void registerAngelScript_image(asIScriptEngine *engine, QImage * img, bool writeable);





/** For immediate script-access to one Geometry instance.
    The usage is easy:
    @code
    // create an instance to wrap a GEOM::Geometry
    GeometryEngineAS script(geometry);
    // compile and run the script (QString)
    script.execute(scriptText);
    @endcode
    To check a script for errors, call createNullEngine(),
    to receive an engine with the full namespace defined
    but without the assigned geometry.
    */
class ImageEngineAS
{
public:
    /** Creates a script engine for the given image */
    ImageEngineAS(QImage * );

    /** Creates a script engine for a new image */
    ImageEngineAS();

    ~ImageEngineAS();

    /** Returns the script engine with all the functionality
        to modify the image instance */
    asIScriptEngine * scriptEngine();

    QImage * image();

    /** Runs the script on the geometry */
    void execute(const QString& script);

private:

    class Private;
    Private * p_;
};








} // namespace MO


#endif // MOSRC_SCRIPT_ANGELSCRIPT_IMAGE_H

#endif // #ifndef MO_DISABLE_ANGELSCRIPT
