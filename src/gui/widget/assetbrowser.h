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

/** Tree-browser for templates, assets, ... */
class AssetBrowser : public QWidget
{
    Q_OBJECT
public:
    explicit AssetBrowser(QWidget *parent = 0);
    ~AssetBrowser();

    /** Returns the directory which is currently selected */
    QString currentDirectory() const;

signals:

    /** Double-click on scene file, should load a scene */
    void sceneSelected(QString filename);

public slots:

    /** Selects one of the short-cut directories */
    void selectDirectory(uint index);
    /** Sets the directory of one of the short-cut buttons */
    void setDirectory(uint index, const QString& dir);

    /** Sets a filter to apply to the displayed files */
    void setFilter(const QString&);

    /** Go up one directory and set as new default */
    void goUp();

    /** Select/double-click action on an item in the browser */
    void doubleClick(const QModelIndex&);

private:

    struct Private;
    Private * p_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_WIDGET_ASSETBROWSER_H
