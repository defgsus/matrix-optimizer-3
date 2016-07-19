/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 4/3/2016</p>
*/

#ifndef MOSRC_GUI_WIDGET_TEXTFINDWIDGET_H
#define MOSRC_GUI_WIDGET_TEXTFINDWIDGET_H

#include <QWidget>


namespace MO {
namespace GUI {

/** A toolbar for common find-text actions */
class TextFindWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TextFindWidget(QWidget *parent = 0);
    ~TextFindWidget();

    QString text() const;

signals:

    void textChanged(const QString& text);
    void nextClick(const QString& text);
    void previousClick(const QString& text);
    void searchEnded();

public slots:

    /** Sets the focus on the text editor */
    void setFocusEdit();
    /** Sets the contents of the text editor */
    void setText(const QString&);

    void findNext();
    void findPrevious();
    void endSearch();

private:
    struct Private;
    Private* p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_TEXTFINDWIDGET_H
