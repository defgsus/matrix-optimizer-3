/** @file doc.h

    @brief mal sehn...

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_DOC_H
#define MOSRC_DOC_H

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
