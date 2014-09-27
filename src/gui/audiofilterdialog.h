/** @file audiofilterdialog.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#ifndef MOSRC_GUI_AUDIOFILTERDIALOG_H
#define MOSRC_GUI_AUDIOFILTERDIALOG_H

#include <QDialog>

class QLabel;

namespace MO {
namespace AUDIO { class MultiFilter; }
namespace GUI {

class FilterResponseWidget;
class SpinBox;

class AudioFilterDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AudioFilterDialog(QWidget *parent = 0);
    ~AudioFilterDialog();

public slots:

private:

    void createWidgets_();
    void updateVisibility_();
    void saveFilter_();
    void restoreFilter_();

    AUDIO::MultiFilter * filter_;
    FilterResponseWidget * display_;

    QLabel * labelOrder_;
    SpinBox * spinOrder_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_AUDIOFILTERDIALOG_H
