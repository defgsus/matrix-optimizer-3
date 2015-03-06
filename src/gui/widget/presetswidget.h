/** @file presetswidget.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.03.2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_PRESETSWIDGET_H
#define MOSRC_GUI_WIDGET_PRESETSWIDGET_H

#include <QWidget>

namespace MO {
namespace GUI {

class FrontPresets;

/** Editor for FrontPresets */
class PresetsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PresetsWidget(QWidget *parent = 0);
    ~PresetsWidget();

    /** Returns the presets class, or NULL if none was assigned */
    FrontPresets * presets() const;

signals:

    /** Emitted when the user changed to another preset */
    void presetSelected(const QString& id);

public slots:

    /** Assigns a list of presets to the view */
    void setPresets(FrontPresets *);

private slots:

    void onComboChanged_();

private:

    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // PRESETSWIDGET_H
