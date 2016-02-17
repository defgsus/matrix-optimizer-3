/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/16/2016</p>
*/

#ifndef MOSRC_GUI_EVOLUTIONDIALOG_H
#define MOSRC_GUI_EVOLUTIONDIALOG_H

#include <QDialog>

namespace MO {
namespace GUI {


class EvolutionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EvolutionDialog(QWidget* parent = 0);
    ~EvolutionDialog();

signals:

public slots:

private:
    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_EVOLUTIONDIALOG_H
