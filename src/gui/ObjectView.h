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
class Parameter;
class ObjectTreeModel;
namespace GL { class Manager; }
namespace GUI {

class ParameterView;
class ObjectListWidget;
class ObjectTreeView;

class ObjectView : public QWidget
{
    Q_OBJECT
public:
    explicit ObjectView(QWidget *parent = 0);
    ~ObjectView();

    /** The currently edited object */
    Object * object() const { return object_; }

signals:

    /** Emitted when an object in the list view was double-clicked
        or a modulation object was created */
    void objectSelected(MO::Object*);

    /** Emitted when the ActivityScope of an object has changed */
    void objectActivityChanged(MO::Object*);

    /** Emitted when statusbar should update */
    void statusTipChanged(const QString&);

public slots:

    /** Sets the object for the view.
        Can be NULL to disable it. */
    void setObject(MO::Object *);

    /** Selects a specific child object in the list view */
    void selectObject(MO::Object *);

    /** Call to change the visibility of a Parameter.
        If the Parameter is not displayed, this function does nothing. */
    void updateParameterVisibility(MO::Parameter*);

    /** Fully recreate the parameter widgets */
    void updateParameters();

    void updateObjectName();

    /** Update texture display */
    void updateImage();

protected slots:

    void infoPopup_();
    void onObjectListSelected(MO::Object *);
    void onObjectListClicked(MO::Object *);

protected:

    void resizeEvent(QResizeEvent *);

private:

    void updateNameLabel_();

    Object * object_;

    GL::Manager* manager_;
    ParameterView * paramView_;
    QVBoxLayout * layout_;
    QToolButton * icon_;
    QLabel * label_, *label2_, *labelImg_;
    /*ObjectListWidget * list_;
    ObjectTreeView* treeView_;
    ObjectTreeModel* treeModel_;*/
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_OBJECT_OBJECTVIEW_H
