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

/** Dialog for displaying and editing a description for the whole scene */
class SceneDescDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SceneDescDialog(QWidget *parent = 0);

    // ---- getter ----

    QString text() const;
    bool showOnStart() const;

signals:

public slots:

    // ---- setter ----

    void setText(const QString& text);
    void setShowOnStart(bool);

    // --- internals ---
protected:

    void closeEvent(QCloseEvent*);
    void keyPressEvent(QKeyEvent *);

private:

    void setChanged_();
    void updateButtons_();

    QPlainTextEdit * p_edit_;
    QCheckBox * p_cb_;
    QPushButton * p_butSave_, * p_butClose_;

    bool p_changed_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_SCENEDESCDIALOG_H
