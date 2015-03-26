/** @file renderdialog.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/25/2015</p>
*/

#ifndef MOSRC_GUI_RENDERDIALOG_H
#define MOSRC_GUI_RENDERDIALOG_H

#include <QDialog>

namespace MO {
namespace GUI {

class RenderDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RenderDialog(QWidget *parent = 0);
    ~RenderDialog();

signals:

public slots:

    void render();

private:

    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_RENDERDIALOG_H
