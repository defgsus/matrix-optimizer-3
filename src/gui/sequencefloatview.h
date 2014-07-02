/** @file sequencefloatview.h

    @brief float sequence editor

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCEFLOATVIEW_H
#define MOSRC_GUI_SEQUENCEFLOATVIEW_H

#include "sequenceview.h"

namespace MO {
class SequenceFloat;
namespace GUI {

class Timeline1DView;
class GeneralSequenceFloatView;

class SequenceFloatView : public SequenceView
{
    Q_OBJECT
public:
    explicit SequenceFloatView(QWidget *parent = 0);

signals:

public slots:

    /** Sets the ViewSpace for the shown sequence */
    void setViewSpace(const UTIL::ViewSpace&);

    void setSequence(MO::SequenceFloat *);

private:

    void createSettingsWidgets_();

    /** Creates a Timeline1DView if not already there. */
    void createTimeline_();
    void createSequenceView_();

    /** Creates another sequence widget if needed */
    void updateSequence_();

    /** Sets settings widgets visibility */
    void updateWidgets_();

    // -------------- MMMEMBER ---------------

    SequenceFloat * sequence_;

    Timeline1DView * timeline_;
    GeneralSequenceFloatView * seqView_;

    QWidget * wOscMode_, * wFreq_, * wPhase_, * wPW_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_SEQUENCEFLOATVIEW_H
