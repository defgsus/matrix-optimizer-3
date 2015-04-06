/** @file scenedescdialog.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.04.2015</p>
*/

#ifndef MOSRC_GUI_SCENEDESCDIALOG_H
#define MOSRC_GUI_SCENEDESCDIALOG_H

#include <QDialog>

class QPlainTextEdit;
class QCheckBox;

namespace MO {
namespace GUI {


class SceneDescDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SceneDescDialog(QWidget *parent = 0);

    QString text() const;
    bool showOnStart() const;

signals:

public slots:

    void setText(const QString& text);
    void setShowOnStart(bool);

private:

    QPlainTextEdit * p_edit_;
    QCheckBox * p_cb_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_SCENEDESCDIALOG_H
