/** @file audioplugindialog.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.05.2015</p>
*/

#ifndef MO_DISABLE_LADSPA

#ifndef MOSRC_GUI_AUDIOPLUGINDIALOG_H
#define MOSRC_GUI_AUDIOPLUGINDIALOG_H

#include <QDialog>

namespace MO {
namespace AUDIO { class LadspaPlugin; }
namespace GUI {

class AudioPluginWidget;

/** Wrapper around AudioPluginWidget */
class AudioPluginDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AudioPluginDialog(QWidget *parent = 0);
    ~AudioPluginDialog();

    /** Currently selected plugin, or NULL */
    AUDIO::LadspaPlugin * currentPlugin();

    /** Static function that opens a modal dialog and
        returns a new instance of the selected plugin,
        or NULL */
    static AUDIO::LadspaPlugin * selectPlugin(QWidget* parent = 0);

signals:

public slots:

private:

    void createWidgets_();

    AudioPluginWidget * plugWidget_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_AUDIOPLUGINDIALOG_H

#endif // #ifndef MO_DISABLE_LADSPA
