/** @file objectinfodialog.h

    @brief Object information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/23/2014</p>
*/

#ifndef OBJECTINFODIALOG_H
#define OBJECTINFODIALOG_H

#include <QDialog>

#include "object/Object_fwd.h"

class QLabel;

namespace MO {
namespace GUI {


class ObjectInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ObjectInfoDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);

    void setObject(Object *);

signals:

public slots:

private:

    QLabel * label_;
};


} // namespace MO
} // namespace GUI


#endif // OBJECTINFODIALOG_H
