/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/13/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_SOUNDFILEDISPLAY_H
#define MOSRC_GUI_WIDGET_SOUNDFILEDISPLAY_H

#include <QWidget>
#include "gui/util/ViewSpace.h"
#include "audio/audio_fwd.h"

namespace MO {
namespace GUI {

class SoundFileDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit SoundFileDisplay(QWidget *parent = 0);
    ~SoundFileDisplay();

    const UTIL::ViewSpace& viewSpace() const;
    AUDIO::SoundFile* soundFile() const;

    int rulerOptions() const;

signals:

    void viewSpaceChanged(const UTIL::ViewSpace&);
    void doubleClicked(Double time);

public slots:

    void setViewSpace(const UTIL::ViewSpace&);
    void setSoundFile(AUDIO::SoundFile* );
    void setRulerOptions(int);
    void fitToSoundFile();

protected:

    void paintEvent(QPaintEvent*) override;

    void mousePressEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;

private:
    struct Private;
    Private* p_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_WIDGET_SOUNDFILEDISPLAY_H
