/** @file generalsequencefloatview.h

    @brief Display for float sequences, except timelines

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_GENERALSEQUENCEFLOATVIEW_H
#define MOSRC_GUI_GENERALSEQUENCEFLOATVIEW_H

#include <QWidget>
#include <QBrush>

#include "gui/util/viewspace.h"

namespace MO {
class SequenceFloat;
class ValueFloatInterface;

namespace GUI {
namespace PAINTER {
    class Grid; class ValueCurve; class ValueCurveData; class SequenceOverpaint;
}


class GeneralSequenceFloatView : public QWidget
{
    Q_OBJECT
public:
    explicit GeneralSequenceFloatView(QWidget *parent = 0);
    ~GeneralSequenceFloatView();

    // --------- getter --------

    const UTIL::ViewSpace& viewSpace() const { return space_; }

    int gridOptions() const;

    // -------- setter ---------

signals:

    /** Send when viewspace was changed by user */
    void viewSpaceChanged(const UTIL::ViewSpace&);

public slots:

    /** Sets the options for the background grid as or-wise combination of PAINTER::Grid::Option */
    void setGridOptions(int options);

    /** Sets a new viewspace for the view */
    void setViewSpace(const UTIL::ViewSpace &, bool send_signal = false);

    /** Sets the sequence to display */
    void setSequence(const MO::SequenceFloat *);

    /** Sets the float interface to display */
    void setValueFloat(const MO::ValueFloatInterface *);

protected:

    void paintEvent(QPaintEvent *);

    const SequenceFloat * sequence_;
    const ValueFloatInterface * valueFloat_;

    UTIL::ViewSpace space_;
    PAINTER::Grid * grid_;
    PAINTER::ValueCurve * curve_;
    PAINTER::ValueCurveData * curveDataSeq_, * curveDataValue_;
    PAINTER::SequenceOverpaint * over_;

    QBrush brushBack_;
};


} // namespace GUI
} // namespace MO

#endif // GENERALSEQUENCEFLOATVIEW_H
