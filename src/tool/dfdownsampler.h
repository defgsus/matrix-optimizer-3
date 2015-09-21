/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/21/2015</p>
*/

#ifndef MOSRC_TOOL_DFDOWNSAMPLER_H
#define MOSRC_TOOL_DFDOWNSAMPLER_H

#include <QThread>
#include <QSize>
#include <QImage>

namespace MO {

/** Distance field image downsampler.
    Modelled after Valve's paper
    "Improved Alpha-Tested Magnification for Vector Textures and Special Effects"
    by Chris Green
    */
class DFDownsampler : public QThread
{
    Q_OBJECT
public:
    DFDownsampler(QObject * parent = 0);
    ~DFDownsampler();

    void setInputImage(const QImage&);
    void setOutputResolution(const QSize&);
    void setSamplingRange(const QSize&);
    void setThreshold(uchar);

    /** Stop processing as soon as possible.
        This will block until the thread has indeed ended. */
    void stop();

    /** Returns the image, valid after QThread::finished() */
    const QImage& getOutputImage() const;

    /** Returns current progress [0,100] */
    int progress() const;

signals:

    /** Emitted when progress() has changed */
    void progressChanged(float);

protected:

    void run() Q_DECL_OVERRIDE;

private:

    struct Private;
    Private * p_;
};

} // namespace MO


#endif // MOSRC_TOOL_DFDOWNSAMPLER_H
