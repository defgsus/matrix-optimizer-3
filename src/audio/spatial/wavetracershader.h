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

/** Impulse response calculation from raymarched distance field */
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
              brightness;
    };

    /** Settings whos change will cause a recompilation */
    struct Settings
    {
        QSize resolution;
        RenderMode renderMode;
        QString userCode;
        uint  maxTraceStep,
              maxReflectStep,
              numMultiSamples;
    };


    WaveTracerShader(QObject * parent = 0);
    ~WaveTracerShader();

    // --------- getter -----------

    /** Returns the current live settings */
    const LiveSettings& liveSettings() const;

    const Settings& settings() const;

    bool wasError() const { return !errorString().isEmpty(); }

    /** Returns a list of errors, or empty string */
    const QString& errorString() const;

    /** Returns the current image, threadsafe.
        Image might be a null image if the renderer is not ready yet. */
    QImage getImage();

    // --------- setter -----------

    /** Applies the new live settings */
    void setLiveSettings(const LiveSettings&);

    /** Applies the new settings */
    void setSettings(const Settings&);

    /** Stops and blocks until finished */
    void stop();

signals:

    /** Emitted when a frame has been rendered.
        Calls to getImage() etc. will be valid after this signal. */
    void frameFinished();

protected:

    void run() Q_DECL_OVERRIDE;

private:

    struct Private;
    Private * p_;
};

} // namespace AUDIO
} // namespace MO

#endif // MOSRC_AUDIO_SPATIAL_WAVETRACERSHADER_H
