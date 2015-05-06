/** @file audiopluginwidget.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MO_DISABLE_LADSPA

#ifndef MOSRC_GUI_WIDGET_AUDIOPLUGINWIDGET_H
#define MOSRC_GUI_WIDGET_AUDIOPLUGINWIDGET_H

#include <QWidget>

namespace MO {
namespace AUDIO { class LadspaPlugin; }
namespace GUI {

/** Audio plugin manager widget.

    Currently for LadspaPlugin,
    as new APIs come in, this will get more general.
*/
class AudioPluginWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AudioPluginWidget(QWidget *parent = 0);
    ~AudioPluginWidget();

    AUDIO::LadspaPlugin * currentPlugin() const;

signals:

public slots:

    void chooseDirectory();

private:

    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_AUDIOPLUGINWIDGET_H

#endif // #ifndef MO_DISABLE_LADSPA
