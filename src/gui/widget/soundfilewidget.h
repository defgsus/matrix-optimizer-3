/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/13/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_SOUNDFILEWIDGET_H
#define MOSRC_GUI_WIDGET_SOUNDFILEWIDGET_H

#include <QWidget>

#include "audio/audio_fwd.h"

namespace MO {
namespace GUI {

class SoundFileWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SoundFileWidget(QWidget *parent = 0);
    ~SoundFileWidget();

signals:

public slots:

    void setSoundFile(AUDIO::SoundFile*);

private:
    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_SOUNDFILEWIDGET_H
