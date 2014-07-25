/** @file sequencefloatview.h

    @brief float sequence editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCEFLOATVIEW_H
#define MOSRC_GUI_SEQUENCEFLOATVIEW_H

#include "sequenceview.h"
#include "object/object_fwd.h"

class QComboBox;

namespace MO {
namespace GUI {
namespace PAINTER { class ValueCurveData; }

class Timeline1DView;
class GeneralSequenceFloatView;
class EquationEditor;
class DoubleSpinBox;
class SpinBox;

class SequenceFloatView : public SequenceView
{
    Q_OBJECT
public:
    explicit SequenceFloatView(QWidget *parent = 0);
    ~SequenceFloatView();

signals:

public slots:

    /** Sets the ViewSpace for the shown sequence */
    void setViewSpace(const UTIL::ViewSpace&);

    void setSequence(MO::SequenceFloat *);

private slots:

    void updateViewSpaceTl_(const UTIL::ViewSpace&);

private:

    void createSettingsWidgets_();

    /** Creates a Timeline1DView if not already there. */
    void createTimeline_();
    void createSequenceView_();

    /** Creates another sequence widget if needed */
    void updateSequence_();

    /** Sets settings widgets visibility */
    void updateWidgets_();

    /** Updates the spinboxes' smallStep value
        and the wavegenerator settings */
    void updatePhaseMode_();

    // -------------- MMMEMBER ---------------

    SequenceFloat * sequence_;

    Timeline1DView * timeline_;
    GeneralSequenceFloatView * seqView_;
    /** For timeline_ */
    PAINTER::ValueCurveData * sequenceCurveData_;


    QWidget * wOscMode_, * wAmp_, * wFreq_, * wPhase_, * wPW_,
            * wEqu_, * wUseFreq_, * wPhaseDeg_,
            * wLoopOverlapping_, * wLoopOverlap_, * wLoopOverlapOffset_,
            * wSpecNum_, * wSpecOct_, * wSpecAmp_, * wSpecPhase_, * wSpecPhaseShift_,
            * wWgOctave_, * wWgOctaveStep_, * wWgPartials_, * wWgSize_,
            * wWgAmp_, * wWgPhase_, * wWgPhaseShift_;
    EquationEditor * wEquEdit_;

    DoubleSpinBox * phaseSpin_, * specPhaseSpin_, * specPhaseShiftSpin_,
            * wgAmp_, * wgPhase_, * wgPhaseShift_;
    SpinBox * wgOctave_, * wgOctaveStep_, * wgPartials_;
    QComboBox * wgSize_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_SEQUENCEFLOATVIEW_H
