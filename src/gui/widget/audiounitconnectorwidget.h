/** @file audiounitconnectorwidget.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef AUDIOUNITCONNECTORWIDGET_H
#define AUDIOUNITCONNECTORWIDGET_H

#include <QWidget>
#include <QPen>
#include <QBrush>
#include "object/object_fwd.h"

namespace MO {
namespace GUI {


class AudioUnitConnectorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AudioUnitConnectorWidget(AudioUnit * au, uint channel, bool isInput, bool isAudio,
                                      QWidget *parent = 0);

    void setModulatorObjectFloat(const ModulatorObjectFloat * m) { modFloat_ = m; }

    AudioUnit * audioUnit() const { return unit_; }
    uint channel() const { return channel_; }
    bool isInput() const { return isInput_; }

signals:

public slots:

protected:

    //int heightForWidth(int w) const Q_DECL_OVERRIDE { return w; }

    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void paintEvent(QPaintEvent *);

private:

    AudioUnit * unit_;
    const ModulatorObjectFloat * modFloat_;

    uint channel_;
    bool isInput_, isAudio_;

    bool hovered_;

    // config

    QBrush brushAudio_, brushMod_, brushModHover_,
            brushModValue_;
    QPen penAudio_, penMod_, penModHover_;

};



} // namespace GUI
} // namespace MO


#endif // AUDIOUNITCONNECTORWIDGET_H
