/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2015</p>
*/

#ifndef MOSRC_GUI_SSWIMPORTER_H
#define MOSRC_GUI_SSWIMPORTER_H

#include <QDialog>

namespace MO {
namespace GUI {

class SswImporter : public QDialog
{
    Q_OBJECT
public:
    explicit SswImporter(QWidget *parent = 0);
    ~SswImporter();

signals:

public slots:

private:
    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_SSWIMPORTER_H
