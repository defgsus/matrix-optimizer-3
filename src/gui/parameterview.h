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

namespace MO {
namespace GUI {

class SpinBox;
class DoubleSpinBox;
class GroupWidget;

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

private slots:

    void updateWidgetValue_(MO::Parameter * );
    void onSequenceChanged(MO::Sequence *);

private:

    void createWidgets_();
    QWidget * createWidget_(Parameter *);
    void clearWidgets_();
    /** update values of parameter widgets */
    void updateWidgetValues_();

    void updateModulatorButton_(Parameter *, QToolButton *);
    void openModulationPopup_(Parameter *, QToolButton *);
    void addRemoveModMenu_(QMenu *, Parameter *);
    void addLinkModMenu_(QMenu *, Parameter *, int objectTypeFlags);
    void addEditModMenu_(QMenu *, Parameter *);

    Scene * scene_;
    Object * object_;
    QList<Parameter*> parameters_;
    QList<QWidget*> widgets_;
    QList<SpinBox*> spinsInt_;
    QList<DoubleSpinBox*> spinsFloat_;
    QList<QComboBox*> combosSelect_;
    QList<QLineEdit*> editsFilename_;
    QWidget * prevEditWidget_;

    QVBoxLayout * layout_;
    GroupWidget * currentGroup_;

    // ------ config ------

    bool doChangeToCreatedTrack_;
};

} // namespace GUI
} // namespace MO


#endif // PARAMETERVIEW_H
