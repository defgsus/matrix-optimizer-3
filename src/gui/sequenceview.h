/** @file sequenceview.h

    @brief Sequence editor base

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCEVIEW_H
#define MOSRC_GUI_SEQUENCEVIEW_H

#include <QWidget>

#include "types/float.h"
#include "object/object_fwd.h"

class QGridLayout;
class QVBoxLayout;
class QScrollArea;
class QCheckBox;

namespace MO {
namespace GUI {
namespace UTIL { class ViewSpace; }

class Ruler;
class DoubleSpinBox;
class TimeBar;
class SceneSettings;

class SequenceView : public QWidget
{
    Q_OBJECT
public:
    explicit SequenceView(QWidget *parent = 0);

    void setScene(Scene * scene);
    Scene * scene() const { return scene_; }

    void setSceneSettings(SceneSettings * s) { sceneSettings_ = s; }
    SceneSettings * sceneSettings() const { return sceneSettings_; }

    /** Returns current viewspace. */
    const UTIL::ViewSpace& viewSpace() const;

signals:

    /** Emitted when the viewspace was changed by user. */
    void viewSpaceChanged(const UTIL::ViewSpace&);

    /** User dragged the time bar */
    void sceneTimeChanged(Double);

    void statusTipChanged(const QString&);

    /** Emitted whenever the user interacts with the view */
    void clicked();

public slots:

    /** Sets the ViewSpace for the shown sequence */
    virtual void setViewSpace(const UTIL::ViewSpace&) = 0;

    /** Tells me that the scene time has changed */
    void setSceneTime(Double);

protected slots:

    /** updates the Rulers to the viewspace. */
    void updateViewSpace_(const UTIL::ViewSpace&);

    void onSequenceChanged_(MO::Sequence *);
    void onParameterChanged_(MO::Parameter *);

    void rulerXClicked_(Double);

protected:
    void resizeEvent(QResizeEvent *);
    void focusInEvent(QFocusEvent * event);

    /** Sets the sequence and creates the default settings. */
    void setSequence_(MO::Sequence *);

    /** Sets the widget that displays the sequence data. */
    void setSequenceWidget_(QWidget *);

    /** Called when something in the sequence has changed. */
    virtual void updateSequence_() = 0;

private:

    QWidget * newContainer_(const QString&);
    QWidget * newDefaultSetting_(const QString & name);

    void clearDefaultSettingsWidgets_();
    void createDefaultSettingsWidgets_();

    Sequence * baseSequence_;
    Scene * scene_;

    SceneSettings * sceneSettings_;

    QGridLayout * grid_;
    Ruler * rulerX_, * rulerY_;
    TimeBar * playBar_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_SEQUENCEVIEW_H
