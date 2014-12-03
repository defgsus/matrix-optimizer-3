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

namespace MO {

class Parameter;
class Object;

namespace GUI {

class SpinBox;
class DoubleSpinBox;



class ParameterWidget : public QFrame
{
    Q_OBJECT
public:
    explicit ParameterWidget(Parameter *, QWidget *parent = 0);

signals:

public slots:

    void updateModulatorButton();
    void openModulationPopup();

    void updateWidgetValue();

protected:

    void dragEnterEvent(QDragEnterEvent*) Q_DECL_OVERRIDE;

private:

    void emitObjectSelected_(Object*);

    void createWidgets_();
    void addRemoveModMenu_(QMenu *, Parameter *);
    void addLinkModMenu_(QMenu *, Parameter *, int objectTypeFlags);
    void addEditModMenu_(QMenu *, Parameter *);
    void addCreateModMenuFloat_(QMenu *, Parameter *);

    Parameter * param_;

    QToolButton * bmod_;

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
