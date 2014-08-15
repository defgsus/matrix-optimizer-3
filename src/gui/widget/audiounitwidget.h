/** @file audiounitwidget.h

    @brief Widget for displaying/connecting AudioUnits

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef AUDIOUNITWIDGET_H
#define AUDIOUNITWIDGET_H

#include <QWidget>
#include "object/object_fwd.h"

namespace MO {
namespace GUI {


class AudioUnitWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AudioUnitWidget(AudioUnit * au, QWidget *parent = 0);

    AudioUnit * audioUnit() const { return unit_; }

signals:

public slots:

private:

    AudioUnit * unit_;
};


} // namespace GUI
} // namespace MO

#endif // AUDIOUNITWIDGET_H
