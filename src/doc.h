/** @file doc.h

    @brief mal sehn...

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_DOC_H
#define MOSRC_DOC_H

/** @todo VIOSO Port
  *
  * - make gfx-time independent of dsp-block-time
  * - fix segfault on delete-object (some views don't realize)
  * - fix multi-track-view edit actions
  * - performance of widget creation in ParameterView
  * - OK fps measure for output window
  * - OK reopen output window when closed
  *     - not on top on windows 10 surface..
  * - OK visibility in geometry: create: obj
  * - OK "Close" instead of "Cancel" in unchanged scene description popup
  * - OK fix AngelScript array (backward compat.)
  * - OK fix drop-into-scene-background problem!!
  *     - OK fix drop-into-scene-background position (always 0,0)
  * - OK populate edit mainmenu with actions from ObjectSceneGraph
  * - OK image loading seems broken for some files
  * - OK fix smoothed camera movement (disable for non-playing)
  * - OK fix line number parser for errors in GLSL
  * - OK attach initialization errors to objects (signal and access in gui)
  *     - better display in gui
  * - OK double-click on scene files in browser should load them
  *
  * minor issues:
  * - when using integrated intel graphics
  *     - lighting does not work
  *     - shader compile error in scene debug renderer
  *
  *
  * workload:
  * 10h compiled for windows (missing libDUMB, libSHP & libGLU)
  * 7h fixed various (gui) things + libjpeg integration
  * 2h nicer asset browser
  * 4h more gui things, and more disappointment with libGLU
  * 3h scene-background-drop (phew...)
  * 4h demo scenes
  * 2h update to documentation
  *
  **/


/** @todo Hamburg impressions:
  *
  * - fullscreen issue (wtf???) and is the stretching hack a solution??
  * - fisheye distortion...
  * - automatic projector camera for clients while dome preview for server
  * - send interface preset along with scene
  * - disable floating dock windows on xubuntu!!!!
  * - file klaus still not 100%
  * - update comes one to late in ProjectorSetupDialog
  *
  */


/** @page random_ideas

    <pre>
    QObject
        Object
            Parameter (active, pos, rotation, audio, etc.)
            ParameterGroup
            Track
                FloatTrack
                EventTrack
            Sequence
                SequenceGroup
                FloatSequence
                EventSequence
            PositionalObject
                SoundSource
                GraphicalObject
                Microphone
                Camera
    </pre>

    @code
    auto o = new MO::Object(root);
    o->addObject( new MO::Object() );
    o->createTrack("name");
    o->createParameter("name");

    auto p = new MO::PositionalObject(o);
    p->createScaleTrack("name");
    p->createRotationTrack();
    p->createPositionTracks();

    @endcode

    <pre>

    Scene
        Camera
            Active
                EventTrack
            Rotation
                FloatTrack
                FloatTrack
            Position
                FloatTrack
                FloatTrack
                FloatTrack
            Microphone
                Rotation
            Microphone
                Rotation
        Sphere
            Position
            SoundSource
                FloatTrack
    </pre>

    <pre>

    Scene
        Context
        Sphere
            QOpenGLFunctions
        Camera
            QOpenGLFunctions
            FrameBuffer

    </pre>

    <pre>
    // ------ updates ------

    // editing
    - Object::addObject / Object::setParentObject
    - Object::deleteObject
    - Object::swapChildren
    - Scene::setParameterValue

    // locks
    - Scene::beginObjectChange ?
    - Scene::beginTreeChange
    - Scene::beginSequenceChange

    // signals to gui
    - Scene::objectChanged ?
    - Scene::objectAdded
    - Scene::treeChanged
    - Scene::sequenceChanged

    </pre>
*/



#endif // MOSRC_DOC_H
