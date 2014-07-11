/** @file parameterview.h

    @brief Display and editor for Object parameters

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/30/2014</p>
*/

#ifndef PARAMETERVIEW_H
#define PARAMETERVIEW_H

#include <QWidget>
#include <QList>
#include <QMap>

class QLabel;
class QVBoxLayout;
namespace MO {
class Object;
class Parameter;
namespace GUI {


class ParameterView : public QWidget
{
    Q_OBJECT
public:
    explicit ParameterView(QWidget *parent = 0);

signals:

    /** When creating a modulator track */
    void objectSelected(MO::Object *);

public slots:

    void setObject(MO::Object * object);

private:

    void createWidgets_();
    QWidget * createWidget_(Parameter *);
    void clearWidgets_();

    Object * object_;
    QList<Parameter*> parameters_;
    QMap<QString, QWidget*> widgets_;
    QWidget * prevEditWidget_;

    QVBoxLayout * layout_;

    // ------ config ------

    bool doChangeToCreatedTrack_;
};

} // namespace GUI
} // namespace MO


#endif // PARAMETERVIEW_H
