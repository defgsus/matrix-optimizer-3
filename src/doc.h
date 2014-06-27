/** @file doc.h

    @brief mal sehn...

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/27/2014</p>
*/

#ifndef MOSRC_DOC_H
#define MOSRC_DOC_H

/** @page random_ideas

    <pre>
    QObject
        Parameter
            FloatParameter
        ParameterGroup
        Track
            FloatTrack
            EventTrack
        Sequence
            FloatSequence
            EventSequence
        SequenceGroup
        Object
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

*/



#endif // MOSRC_DOC_H
