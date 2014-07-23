/** @file objectview.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/1/2014</p>
*/

#ifndef MOSRC_OBJECT_OBJECTVIEW_H
#define MOSRC_OBJECT_OBJECTVIEW_H

#include <QWidget>

class QVBoxLayout;
class QToolButton;
class QLabel;

namespace MO {
class Object;
namespace GUI {

class ParameterView;

class ObjectView : public QWidget
{
    Q_OBJECT
public:
    explicit ObjectView(QWidget *parent = 0);

signals:

    /** When a modulation track was created */
    void objectSelected(MO::Object*);

    /** Emitted when the ActivityScope of an object has changed */
    void objectActivityChanged(MO::Object*);

    /** Emitted when statusbar should update */
    void statusTipChanged(const QString&);

public slots:

    /** Sets the object for the view.
        Can be NULL to disable it. */
    void setObject(MO::Object *);

protected slots:

    void infoPopup_();

protected:

    void resizeEvent(QResizeEvent *);

private:

    void updateNameLabel_();

    Object * object_;

    ParameterView * paramView_;
    QVBoxLayout * layout_;
    QToolButton * icon_;
    QLabel * label_, *label2_;

};

} // namespace GUI
} // namespace MO

#endif // MOSRC_OBJECT_OBJECTVIEW_H
