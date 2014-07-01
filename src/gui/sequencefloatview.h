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
namespace GUI {

class Timeline1DView;

class SequenceFloatView : public SequenceView
{
    Q_OBJECT
public:
    explicit SequenceFloatView(QWidget *parent = 0);

signals:

public slots:

    /** Sets the ViewSpace for the shown sequence */
    void setViewSpace(const UTIL::ViewSpace&);

private:

    Timeline1DView * timeline_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_SEQUENCEFLOATVIEW_H
