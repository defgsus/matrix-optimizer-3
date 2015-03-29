/** @file diskrenderer.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/27/2015</p>
*/

#ifndef MOSRC_ENGINE_DISKRENDERER_H
#define MOSRC_ENGINE_DISKRENDERER_H

#include <QThread>
#include <QString>


namespace MO {

class Scene;
class DiskRenderSettings;

/** Disk render thread */
class DiskRenderer : public QThread
{
    Q_OBJECT
public:
    explicit DiskRenderer(QObject *parent = 0);
    ~DiskRenderer();

    bool ok() const;
    QString errorString() const;

    const DiskRenderSettings& settings() const;

    /** Returns a multi-line string with progress information
        about the current rendering.
        Display this on receiving the progress() signal. */
    QString progressString() const;

signals:

    /** Periodically emitted */
    void progress(int percent);

public slots:

    void setSettings(const DiskRenderSettings&);

    /** Assigns a scene filename for loading */
    void setSceneFilename(const QString& fn);

    /** Loads a scene and constructs everything. */
    bool loadScene(const QString& fn);

    /* Assigns a scene. The gl context will be reinitialized. */
    //void setScene(Scene * );

    /** Request stop and block */
    void stop();

protected:

    void run() Q_DECL_OVERRIDE;

private:

    struct Private;
    Private * p_;
};

} // namespace MO


#endif // MOSRC_ENGINE_DISKRENDERER_H
