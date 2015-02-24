/** @file frontitemeditor.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 24.02.2015</p>
*/

#ifndef FRONTITEMEDITOR_H
#define FRONTITEMEDITOR_H

#include <QWidget>

class QLabel;

namespace MO {
namespace GUI {

class AbstractFrontItem;
class PropertiesView;

/** Editor for AbstractFrontItem */
class FrontItemEditor : public QWidget
{
    Q_OBJECT
public:
    explicit FrontItemEditor(QWidget *parent = 0);

signals:

public slots:

    /** Sets the item to edit.
        The item must stay valid until setItem is called
        with another valid item or NULL to clear the view. */
    void setItem(AbstractFrontItem * item);

private slots:

    void onPropertyChanged_(const QString& id);

private:

    void createWidgets_();
    void updateWidgets_();

    AbstractFrontItem * p_item_;
    PropertiesView * p_props_;
    QLabel * p_label_;
};


} // namespace GUI
} // namespace MO


#endif // FRONTITEMEDITOR_H
