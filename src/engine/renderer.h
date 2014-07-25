/** @file renderer.h

    @brief To-disk renderer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/25/2014</p>
*/

#ifndef MOSRC_ENGINE_RENDERER_H
#define MOSRC_ENGINE_RENDERER_H

#include <QThread>

#include "types/float.h"
#include "object/object_fwd.h"

namespace MO {

class Renderer : public QThread
{
    Q_OBJECT
public:
    explicit Renderer(QObject *parent = 0);

signals:

    void progress(int percent, Double estimated, Double left);

public slots:

    void setScene(Scene * scene) { scene_ = scene; }
    void setOutputPath(const QString& path) { path_ = path; }

    bool prepareRendering();

protected:

    void run() Q_DECL_OVERRIDE;

private:

    //void createPaths_();

    QString path_;

    Scene * scene_;
};

} // namespace MO


#endif // MOSRC_ENGINE_RENDERER_H
