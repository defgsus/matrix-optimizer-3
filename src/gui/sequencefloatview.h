/** @file sequencefloatview.h

    @brief float sequence editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCEFLOATVIEW_H
#define MOSRC_GUI_SEQUENCEFLOATVIEW_H

#include <QWidget>

#include "object/object_fwd.h"
#include "util/viewspace.h"

class QLayout;

namespace MO {
namespace GUI {
namespace PAINTER { class ValueCurveData; }

class Timeline1DView;
class GeneralSequenceFloatView;
class SequenceView;

class SequenceFloatView : public QWidget
{
    Q_OBJECT
public:
    explicit SequenceFloatView(SequenceView *parent = 0);
    ~SequenceFloatView();

    SequenceFloat * sequence() const { return sequence_; }

signals:

    void viewSpaceChanged(const UTIL::ViewSpace&);

public slots:

    /** Sets the ViewSpace for the shown sequence */
    void setViewSpace(const UTIL::ViewSpace&);

    void setSequence(SequenceFloat *);

    /** Creates another sequence widget if needed */
    void updateSequence();

private slots:

    void updateViewSpaceTl_(const UTIL::ViewSpace&);

private:

    /** Creates a Timeline1DView if not already there. */
    void createTimeline_();
    void createSequenceView_();

    // -------------- MMMEMBER ---------------

    SequenceView * view_;
    SequenceFloat * sequence_;

    Timeline1DView * timeline_;
    GeneralSequenceFloatView * seqView_;
    /** For timeline_ */
    PAINTER::ValueCurveData * sequenceCurveData_;

    QLayout * layout_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_SEQUENCEFLOATVIEW_H
