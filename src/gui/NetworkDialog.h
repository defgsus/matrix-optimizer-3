/** @file networkdialog.h

    @brief Network config dialog

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/6/2014</p>
*/

#ifndef MOSRC_GUI_NETWORKDIALOG_H
#define MOSRC_GUI_NETWORKDIALOG_H

#include <QDialog>

class QTableWidget;

namespace MO {
namespace GUI {


class NetworkDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NetworkDialog(QWidget *parent = 0);

    /** Returns name for QNetworkConfiguration::Purpose enum */
    static QString purposeName(int purpose);

    /** Returns name for QNetworkConfiguration::Type enum */
    static QString typeName(int purpose);

    /** Returns name for QNetworkConfiguration::StateFlags enum */
    static QString stateName(int purpose);

signals:

public slots:

    void rescan();

private:

    void createWidgets_();

    QTableWidget * table_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_NETWORKDIALOG_H
