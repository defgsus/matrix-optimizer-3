/** @file objecttreeviewoverpaint.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/18/2014</p>
*/

#ifndef MOSRC_GUI_PAINTER_OBJECTTREEVIEWOVERPAINT_H
#define MOSRC_GUI_PAINTER_OBJECTTREEVIEWOVERPAINT_H

#include <vector>

#include <QWidget>
#include <QModelIndex>
#include <QMap>

namespace MO {
class Object;
namespace GUI {

class ObjectTreeView;

class ObjectTreeViewOverpaint : public QWidget
{
    Q_OBJECT
public:
    explicit ObjectTreeViewOverpaint(ObjectTreeView *parent = 0);

signals:

public slots:

protected:

    void paintEvent(QPaintEvent *);

private:

    void getIndexMap_();
    void getIndexMap_(Object * parent);

    void getModulations_();
    void getModulations_(Object * parent);

    ObjectTreeView * view_;

    QMap<QString, QModelIndex> indexMap_;

    struct ModPath_
    {
        QModelIndex from, to;
        ModPath_(const QModelIndex& from, const QModelIndex& to)
            : from(from), to(to) { }
    };

    std::vector<ModPath_> modPaths_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_PAINTER_OBJECTTREEVIEWOVERPAINT_H
