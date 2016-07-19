/** @file envelopewidget.h

    @brief A display for a number of levels

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/8/2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_ENVELOPEWIDGET_H
#define MOSRC_GUI_WIDGET_ENVELOPEWIDGET_H

#include <vector>

#include <QWidget>

#include "types/float.h"

namespace MO {
namespace GUI {


class EnvelopeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EnvelopeWidget(QWidget *parent = 0);

    uint getNumberChannels() const { return numChannels_; }

    bool isAnimating() const { return doAnimate_; }

signals:

public slots:

    void setNumberChannels(uint num);

    void setAnimating(bool enable) { doAnimate_ = enable; if (doAnimate_) update(); }

    void setLevel(uint index, F32 value);
    void setLevel(const F32 * levels);

protected:

    void paintEvent(QPaintEvent *);

private:

    uint numChannels_;
    std::vector<F32> level_;

    bool doAnimate_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_ENVELOPEWIDGET_H
