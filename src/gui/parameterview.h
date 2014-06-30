/** @file parameterview.h

    @brief Display and editor for Object parameters

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef PARAMETERVIEW_H
#define PARAMETERVIEW_H

#include <QWidget>

namespace MO {
namespace GUI {


class ParameterView : public QWidget
{
    Q_OBJECT
public:
    explicit ParameterView(QWidget *parent = 0);

signals:

public slots:

};

} // namespace GUI
} // namespace MO


#endif // PARAMETERVIEW_H
