/** @file wavetracershader.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 27.04.2015</p>
*/

#ifndef MOSRC_AUDIO_SPATIAL_WAVETRACERSHADER_H
#define MOSRC_AUDIO_SPATIAL_WAVETRACERSHADER_H

#include <QThread>
#include <QImage>

#include "types/vector.h"

namespace MO {
namespace AUDIO {

class IrMap;

/** Impulse response calculation from raymarched distance field.
    Does work in it's own thread and wraps everything nicely. */
class WaveTracerShader : public QThread
{
    Q_OBJECT
public:

    enum RenderMode
    {
        RM_WAVE_TRACER          = 0,
        RM_WAVE_TRACER_VISIBLE  = 1,
        RM_RAY_TRACER           = 2,
        RM_FIELD_SLICE          = 3
    };

    /** Settings that can be changed without recompilation */
    struct LiveSettings
    {
        Mat4 camera;
        Vec3 soundPos,
             soundColor;

        Float fudge,
              normalEps,
              maxTraceDist,
              micAngle,
              soundRadius,
              reflectivness,
              brightness,
              diffuse,
              diffuseRnd,
              fresnel,
              rndRay;

        bool doFlipPhase;
    };

    /** Settings whos change will cause a recompilation */
    struct Settings
    {
        QSize resolution;
        RenderMode renderMode;
        QString userCode;
        uint  maxTraceStep,
              maxReflectStep,
              numPasses; //! This may cause the thread to stop but not to restart
        bool doPassAverage; //! To be set on start!
    };


    WaveTracerShader(QObject * parent = 0);
    ~WaveTracerShader();

    // --------- getter -----------

    /** Returns the current live settings */
    const LiveSettings& liveSettings() const;

    const Settings& settings() const;

    QString infoString() const;

    /** Returns the current pass */
    uint passCount() const;

    bool wasError() const { return !errorString().isEmpty(); }

    /** Returns a list of errors, or empty string */
    const QString& errorString() const;

    /** Returns information about the current impulse response sampler */
    QString getIrInfo() const;

    /** Returns a copy of the impulse response map, threadsafe */
    AUDIO::IrMap getIrMap() const;

    /** Returns the current image, threadsafe.
        Image might be a null image if the renderer is not ready yet. */
    QImage getImage();

    /** Returns the current impulse response as image, threadsafe */
    QImage getIrImage(const QSize& s);

    // --------- setter -----------

    /** Applies the new live settings, threadsafe */
    void setLiveSettings(const LiveSettings&);

    /** Applies the new settings, threadsafe */
    void setSettings(const Settings&);

public slots:

    /** Stops and blocks until finished, threadsafe */
    void stop();

    /** Requests an impulse response image to be created on another thread.
        Emits finishedIrImage(). */
    void requestIrImage(const QSize& s);

    /* Requests a rendered image to be created on another thread.
        Emits finishedImage().*/
    //void requestImage(const QSize& s);

signals:

    /** Emitted when a frame has been rendered.
        Calls to getImage() etc. will be valid after this signal. */
    void frameFinished();

    /* Answer to requestImage() */
    //void finishedImage(QImage img);

    /** Answer to requestIrImage() */
    void finishedIrImage(QImage img);

protected:

    void run() Q_DECL_OVERRIDE;

private:

    struct Private;
    Private * p_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_SPATIAL_WAVETRACERSHADER_H
