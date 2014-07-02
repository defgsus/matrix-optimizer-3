/** @file sequenceview.h

    @brief Sequence editor base

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCEVIEW_H
#define MOSRC_GUI_SEQUENCEVIEW_H

#include <QWidget>

class QGridLayout;
class QVBoxLayout;
class QScrollArea;
class QCheckBox;

namespace MO {
class Sequence;
namespace GUI {
namespace UTIL { class ViewSpace; }

class Ruler;
class DoubleSpinBox;

class SequenceView : public QWidget
{
    Q_OBJECT
public:
    explicit SequenceView(QWidget *parent = 0);

    /** Creates a new setting widget container.
        Add your stuff to the returned widget's layout. */
    QWidget * newSetting(const QString & name);

signals:

    /** Emitted when the viewspace was changed by user. */
    void viewSpaceChanged(const UTIL::ViewSpace&);

public slots:

    /** Sets the ViewSpace for the shown sequence */
    virtual void setViewSpace(const UTIL::ViewSpace&) = 0;

protected slots:

    /** updates the Rulers to the viewspace. */
    void updateViewSpace_(const UTIL::ViewSpace&);

    void sequenceTimeChanged(MO::Sequence *);

protected:

    /** This resizes the scrollarea viewport to minimum.
        Call this after changes to setting widgets visibilty. */
    void squeezeView_();

    /** Sets the sequence and creates the default settings. */
    void setSequence_(MO::Sequence *);

    /** Sets the widget that displays the sequence data. */
    void setSequenceWidget_(QWidget *);

    /** Clear all settings widgets */
    void clearSettingsWidgets_();

    /** Adds a custom widget to the settings. @see newSetting() */
    void addSettingsWidget_(QWidget *);

private:

    QWidget * newContainer_(const QString&);
    QWidget * newDefaultSetting_(const QString & name);

    void clearDefaultSettingsWidgets_();
    void createDefaultSettingsWidgets_();

    Sequence * baseSequence_;

    QGridLayout * grid_;
    QVBoxLayout * settingsLayout_;
    Ruler * rulerX_, * rulerY_;
    QScrollArea * settings_;
    QWidget * defaultSettingsContainer_,
            * customSettingsContainer_;
    QList<QWidget*> defaultSettingsWidgets_,
                    customSettingsWidgets_;

    DoubleSpinBox * spinStart_, * spinLength_, * spinEnd_,
        * spinLoopStart_, * spinLoopLength_, * spinLoopEnd_,
        * spinTimeOffset_, * spinSpeed_;
    QWidget * wLoopStart_, * wLoopLength_, * wLoopEnd_;

    QCheckBox * cbLooping_;

    bool defaultSettingsAvailable_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_SEQUENCEVIEW_H
