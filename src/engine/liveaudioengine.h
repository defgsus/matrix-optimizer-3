/** @file liveaudioengine.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 02.12.2014</p>
*/

#ifndef MOSRC_ENGINE_LIVEAUDIOENGINE_H
#define MOSRC_ENGINE_LIVEAUDIOENGINE_H

#include <QObject>

namespace MO {

class Scene;

class LiveAudioEngine : public QObject
{
    Q_OBJECT
public:
    explicit LiveAudioEngine(QObject *parent = 0);
    ~LiveAudioEngine();

signals:

public slots:

    /** Creates the dsp path for the given scene */
    void setScene(Scene *);

private:

    class Private;
    Private * p_;

};


} // namespace MO

#endif // MOSRC_ENGINE_LIVEAUDIOENGINE_H
