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

    /** Returns currently selected preset id, or empty string */
    QString currentPresetId() const;

signals:

    /** Emitted when the user changed to another preset */
    void presetLoaded(const QString& id);

    /** Emitted, when the current preset should be saved. */
    void presetSaveRequest(const QString& id);

    /** Emitted, when the current preset should be loaded. */
    void presetLoadRequest(const QString& id);

public slots:

    /** Assigns a list of presets to the view */
    void setPresets(FrontPresets *);

    /** Selects a preset in the combobox, if present. */
    void selectPreset(const QString& id);

    /** Selects the previous preset and emits presetLoadRequest() */
    void loadPrevious();
    /** Selects the next preset and emits presetLoadRequest() */
    void loadNext();

    /** Updates the list of presets */
    void updatePresets();

private slots:

    void onComboChanged_();

private:

    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // PRESETSWIDGET_H
