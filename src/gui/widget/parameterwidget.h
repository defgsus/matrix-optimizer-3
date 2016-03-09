/** @file parameterwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.12.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_PARAMETERWIDGET_H
#define MOSRC_GUI_WIDGET_PARAMETERWIDGET_H

#include <QFrame>

class QToolButton;
class QMenu;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QHBoxLayout;

namespace MO {

class Parameter;
class ParameterTexture;
class Object;
class ObjectEditor;

namespace GUI {

class SpinBox;
class DoubleSpinBox;



class ParameterWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ParameterWidget(Parameter *, QWidget *parent = 0);

signals:

    void objectSelected(MO::Object*);
    void statusTipChanged(const QString&);

public slots:

    void openModulationPopup();
    void openVisibilityPopup();

    void updateButtons();
    void updateWidgetValue();

protected:

    void dragEnterEvent(QDragEnterEvent*) Q_DECL_OVERRIDE;
    void dropEvent(QDropEvent *) Q_DECL_OVERRIDE;

private:

    void emitObjectSelected_(Object*);
    void emitStatusTipChanged_(const QString&);

    void createWidgets_();
    void addTexParamButtons_(ParameterTexture*, QHBoxLayout*);
    void addRemoveModMenu_(QMenu *, Parameter *);
    void addLinkModMenu_(QMenu *, Parameter *, int objectTypeFlags);
    void addEditModMenu_(QMenu *, Parameter *);
    void addCreateModMenuFloat_(QMenu *, Parameter *);

    Parameter * param_;
    ObjectEditor * editor_;

    QToolButton * bmod_, *bvis_, *btexmm_;

    SpinBox * spinInt_;
    DoubleSpinBox * spinFloat_;
    QComboBox * comboSelect_;
    QCheckBox * checkBox_;
    QLineEdit * lineEdit_;

    // ------ config ------

    bool doChangeToCreatedMod_;

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_PARAMETERWIDGET_H
