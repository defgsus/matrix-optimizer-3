/** @file assetbrowser.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2015</p>
*/

#ifndef MOSRC_GUI_WIDGET_ASSETBROWSER_H
#define MOSRC_GUI_WIDGET_ASSETBROWSER_H

#include <QWidget>

namespace MO {
namespace GUI {

/** List-browser for templates, assets, ... */
class AssetBrowser : public QWidget
{
    Q_OBJECT
public:
    explicit AssetBrowser(QWidget *parent = 0);
    ~AssetBrowser();

signals:

public slots:

    /** Selects one of the short-cut directories */
    void selectDirectory(uint index);
    void setDirectory(uint index, const QString& dir);
protected:

    void mouseDoubleClickEvent(QMouseEvent*) Q_DECL_OVERRIDE;

private:

    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_ASSETBROWSER_H
