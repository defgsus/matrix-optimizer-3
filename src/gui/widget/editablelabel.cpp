/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/9/2016</p>
*/

#include <QMouseEvent>
#include <QLineEdit>

#include "editablelabel.h"

namespace MO {
namespace GUI {


EditableLabel::EditableLabel(const QString& text, QWidget *parent)
    : QLabel        (text, parent)
    , p_editor_     (0)
{
    setMouseTracking(true);
    setProperty("editable", true);
}

void EditableLabel::mouseDoubleClickEvent(QMouseEvent* )
{
    openEditor();
}

void EditableLabel::openEditor()
{
    if (p_editor_)
    {
        p_editor_->raise();
        return;
    }

    p_editor_ = new QLineEdit(text(), this);
    p_editor_->setGeometry(rect());
    p_editor_->setAttribute(Qt::WA_DeleteOnClose);

    connect(p_editor_, &QLineEdit::editingFinished, [=]()
    {
        setText(p_editor_->text());
        emit editingFinished();
        p_editor_->close();
    });

    connect(p_editor_, &QLineEdit::destroyed, [=]() { p_editor_ = nullptr; });

    p_editor_->show();
    p_editor_->setFocus();
}


} // namespace GUI
} // namespace MO
