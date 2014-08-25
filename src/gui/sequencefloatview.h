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

    // -------------- MMMEMBER ---------------

    SequenceFloat * sequence_;

    Timeline1DView * timeline_;
    GeneralSequenceFloatView * seqView_;
    /** For timeline_ */
    PAINTER::ValueCurveData * sequenceCurveData_;

};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_SEQUENCEFLOATVIEW_H
