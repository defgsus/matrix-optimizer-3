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

    /** Number of bands to display/analyze */
    uint numBands() const { return numBands_; }
    /** Returns the length of the buffer for analyzing bands */
    uint bufferSize() const { return bufferSize_; }

    /** Returns x coordinate for filter frequency */
    int XForFreq(F32 freq) const;
    /** Returns frequency for x coordinate */
    F32 freqForX(int x) const;
    /** Returns frequency of given band */
    F32 freqForBand(uint band) const;

    /** Returns the current filter settings */
    const AUDIO::MultiFilter & filter() const { return *filter_; }

signals:

    /** Emitted on clicking or dragging on the x-axis */
    void frequencyChanged(F32 freq);

public slots:

    /** Sets a new filter setting and updates display */
    void setFilter(const AUDIO::MultiFilter&, bool update = true);

    /** Sets the number of bands to display/analyze */
    void setNumBands(uint num, bool update = true);
    /** Sets the length of the buffer for analyzing bands */
    void setBufferSize(uint size, bool update = true);

protected:

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);

    void paintEvent(QPaintEvent *);

private:

    void calcResponse_();

    AUDIO::MultiFilter * filter_;
    std::vector<F32> response_;

    uint sampleRate_, numBands_, bufferSize_;
    F32 lowFreq_;
    // not implemented yet
    bool logScale_;

    QThread * thread_;

    // -- config --
    QPen penGrid_, penFreq_, penCurve_;
    QBrush brushBack_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_FILTERRESPONSEWIDGET_H
