/** @file filterresponsewidget.h

    @brief MultiFilter response display

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 26.09.2014</p>
*/

#ifndef MOSRC_GUI_WIDGET_FILTERRESPONSEWIDGET_H
#define MOSRC_GUI_WIDGET_FILTERRESPONSEWIDGET_H

#include <vector>

#include <QWidget>
#include <QPen>
#include <QBrush>

#include "types/float.h"

namespace MO {
namespace AUDIO { class MultiFilter; }
namespace GUI {

class FilterResponseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FilterResponseWidget(QWidget *parent = 0);
    ~FilterResponseWidget();

    uint sampleRate() const { return sampleRate_; }

    int XForFreq(F32 freq) const;
    F32 freqForX(int x) const;
    F32 freqForBand(uint band) const;

signals:

    /** Emitted on clicking or dragging on the x-axis */
    void frequencyChanged(F32 freq);

public slots:

    /** Sets a new filter setting and updates display */
    void setFilter(const AUDIO::MultiFilter&);

protected:

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);

private:

    void calcResponse_();

    AUDIO::MultiFilter * filter_;
    std::vector<F32> response_;

    uint sampleRate_;
    F32 lowFreq_;
    bool logScale_;

    // -- config --
    QPen penGrid_, penFreq_, penCurve_;
    QBrush brushBack_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_FILTERRESPONSEWIDGET_H
