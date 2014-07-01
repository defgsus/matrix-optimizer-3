/** @file sequenceview.h

    @brief Sequence editor base

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_GUI_SEQUENCEVIEW_H
#define MOSRC_GUI_SEQUENCEVIEW_H

#include <QWidget>

class QGridLayout;
class QVBoxLayout;
class QScrollArea;

namespace MO {
namespace GUI {
namespace UTIL { class ViewSpace; }

class Ruler;

class SequenceView : public QWidget
{
    Q_OBJECT
public:
    explicit SequenceView(QWidget *parent = 0);

signals:

    /** Emitted when the viewspace was changed by user. */
    void viewSpaceChanged(const UTIL::ViewSpace&);

public slots:

    /** Sets the ViewSpace for the shown sequence */
    virtual void setViewSpace(const UTIL::ViewSpace&) = 0;

protected slots:

    /** updates the Rulers to the viewspace. */
    void updateViewSpace_(const UTIL::ViewSpace&);

protected:

    void setSequenceWidget_(QWidget *);

private:

    void createDefaultSettings_();

    QGridLayout * grid_;
    QVBoxLayout * settingsLayout_;
    Ruler * rulerX_, * rulerY_;
    QScrollArea * settings_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_SEQUENCEVIEW_H
