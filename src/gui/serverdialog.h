/** @file serverdialog.h

    @brief Dialog for controlling clients

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 10.09.2014</p>
*/

#ifndef MOSRC_GUI_SERVERDIALOG_H
#define MOSRC_GUI_SERVERDIALOG_H

#include <QDialog>

#include "network/network_fwd.h"


class QCheckBox;

namespace MO {
namespace GUI {


class ServerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ServerDialog(QWidget *parent = 0);
    ~ServerDialog();

signals:

public slots:

private slots:

    void startServer_(bool);

private:
    void createWidgets_();

    ServerEngine * server_;

    QCheckBox * cbRunning_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_SERVERDIALOG_H
