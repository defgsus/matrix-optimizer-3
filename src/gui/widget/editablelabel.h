/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/9/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_EDITABLELABEL_H
#define MOSRC_GUI_WIDGET_EDITABLELABEL_H

#include <QLabel>
class QLineEdit;

namespace MO {
namespace GUI {

class EditableLabel
        : public QLabel
{
    Q_OBJECT
public:
    explicit EditableLabel(const QString& text, QWidget *parent = 0);
    explicit EditableLabel(QWidget *parent = 0)
        : EditableLabel(QString(), parent) { }

signals:

    void editingFinished();

public slots:

    void openEditor();

protected:

    void mouseDoubleClickEvent(QMouseEvent*) Q_DECL_OVERRIDE;

private:
    QLineEdit* p_editor_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_EDITABLELABEL_H
