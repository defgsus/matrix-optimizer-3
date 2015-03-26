/** @file parameterview.h

    @brief Display and editor for Object parameters

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef PARAMETERVIEW_H
#define PARAMETERVIEW_H

#include <QWidget>
#include <QList>
#include <QMap>

#include "object/object_fwd.h"

class QMenu;
class QLabel;
class QVBoxLayout;
class QToolButton;
class QComboBox;
class QLineEdit;
class QScrollArea;
class QCheckBox;

namespace MO {
namespace GUI {

class SpinBox;
class DoubleSpinBox;
class GroupWidget;
class ParameterWidget;

class ParameterView : public QWidget
{
    Q_OBJECT
public:
    explicit ParameterView(QWidget *parent = 0);

signals:

    /** When creating a modulator track */
    void objectSelected(MO::Object *);

    /** Emitted when the ActivityScope of an object has changed */
    void objectActivityChanged(MO::Object*);

    /** Emitted when statusbar should update */
    void statusTipChanged(const QString&);

public slots:

    void setObject(MO::Object * object);

    /** Call to change the visibility of a Parameter.
        If the Parameter is not displayed, this function does nothing. */
    void updateParameterVisibility(MO::Parameter *);

private slots:

    void onSequenceChanged(MO::Sequence *);
    void updateWidgetValue_(MO::Parameter*);

protected:

    void resizeEvent(QResizeEvent *);

private:

    GroupWidget * getGroupWidget_(const Parameter*);

    void squeezeView_();
    void createWidgets_();
    void clearWidgets_();
    /** update values of parameter widgets */
    void updateWidgetValues_();


    Scene * scene_;
    ObjectEditor * editor_;
    Object * object_;
    QList<Parameter*> parameters_;
    QList<QWidget*> widgets_;
    QMap<QString, GroupWidget*> groups_;
    QMap<Parameter*, ParameterWidget*> paramMap_;

    QVBoxLayout * layout_;
    QScrollArea * scrollArea_;
    QWidget * container_;

};

} // namespace GUI
} // namespace MO


#endif // PARAMETERVIEW_H
