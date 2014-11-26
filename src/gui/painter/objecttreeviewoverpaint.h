/** @file objecttreeviewoverpaint.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/18/2014</p>
*/

#ifndef MO_DISABLE_TREE

#ifndef MOSRC_GUI_PAINTER_OBJECTTREEVIEWOVERPAINT_H
#define MOSRC_GUI_PAINTER_OBJECTTREEVIEWOVERPAINT_H

#include <vector>

#include <QWidget>
#include <QModelIndex>
#include <QMap>
#include <QPainterPath>
#include <QPen>

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

    void updateAll();

protected:

    void paintEvent(QPaintEvent *);

private:

    void getIndexMap_();
    void getIndexMap_(Object * parent);

    void updateIndexMapVisual_();

    void getModulations_();
    void getModulations_(Object * parent);

    ObjectTreeView * view_;

    QMap<QString, QModelIndex> indexMap_;

    struct ModPath_
    {
        QModelIndex from, to;
        QPainterPath path;
        ModPath_(const QModelIndex& from, const QModelIndex& to)
            : from(from), to(to) { }
    };

    std::vector<ModPath_> modPaths_;

    // --- config ---

    QPen penPath_, penPathSel_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_PAINTER_OBJECTTREEVIEWOVERPAINT_H

#endif
