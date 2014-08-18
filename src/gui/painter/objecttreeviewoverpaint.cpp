/** @file objecttreeviewoverpaint.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/18/2014</p>
*/
#include <QDebug>
#include <QPainter>
#include <QPainterPath>

#include "objecttreeviewoverpaint.h"
#include "gui/objecttreeview.h"
#include "object/object.h"
#include "object/scene.h"
#include "model/objecttreemodel.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {

ObjectTreeViewOverpaint::ObjectTreeViewOverpaint(ObjectTreeView *parent) :
    QWidget (parent),
    view_   (parent)
{
    setAttribute(Qt::WA_TransparentForMouseEvents);
}

void ObjectTreeViewOverpaint::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    p.setPen(QPen(QColor(0,0,0)));
    /*p.drawLine(0,0,width(),height());
    p.drawLine(0,height(),width(),0);*/

    getIndexMap_();
    getModulations_();

    MO_DEBUG("modpaths = " << modPaths_.size());
    for (const ModPath_& m : modPaths_)
    {
        const QRect
                from = view_->visualRect(m.from),
                to = view_->visualRect(m.to);
        qDebug() << from << to;
        QPainterPath path(QPoint(from.left(), from.top() + from.height() / 2));
        path.cubicTo(from.left() + 30, from.top() + from.height() / 2,
                     to.left() + 30, to.top() + to.height() / 2,
                     to.left(), to.top() + to.height() / 2);
        p.drawPath(path);
    }
}

void ObjectTreeViewOverpaint::getIndexMap_()
{
    indexMap_.clear();
    if (view_->sceneObject())
        getIndexMap_(view_->sceneObject());
}

void ObjectTreeViewOverpaint::getIndexMap_(Object * parent)
{
    for (auto obj : parent->childObjects())
    {
        // find QModelIndex

        QModelIndex idx = view_->findIndexForObject(obj);

        if (idx.isValid())
            indexMap_.insert(obj->idName(), idx);

        // go through childs
        getIndexMap_(obj);
    }

    /*
    int row = 0;
    while (true)
    {
        QModelIndex s = parent.child(row, 0);
        if (!s.isValid())
            break;

        ++row;

        Object * obj = s.data(ObjectRole).value<Object*>();
        MO_ASSERT(obj, "no object for itemindex " << s);

        indexMap_.insert(obj->idName(), s);

        getIndexMap_(s);
    }
    */
}

void ObjectTreeViewOverpaint::getModulations_()
{
    modPaths_.clear();
    getModulations_(view_->sceneObject());
}

void ObjectTreeViewOverpaint::getModulations_(Object * parent)
{
    for (auto obj : parent->childObjects())
    {
        auto i = indexMap_.find(obj->idName());
        if (i == indexMap_.end())
        {
            MO_WARNING("object '" << obj->idName() << "' not found in indexMap_");
            continue;
        }
        const QModelIndex idx = i.value();

        QList<Object*> mods = obj->getModulatingObjects();
        //MO_DEBUG(mods.size() << " mods for '" << obj->idName() << "'");
        for (auto m : mods)
        {
            i = indexMap_.find(m->idName());
            if (i == indexMap_.end())
            {
                MO_WARNING("modulator '" << m->idName() << "' not found in indexMap_");
                continue;
            }

            modPaths_.push_back(ModPath_(i.value(), idx));
        }

        // go through childs
        getModulations_(obj);
    }


    /*
    int row = 0;
    while (true)
    {
        MO_DEBUG("scan row " << row << " in " << parent);
        QModelIndex s = parent.child(row, 0);
        if (!s.isValid())
            break;

        ++row;

        Object * obj = s.data(ObjectRole).value<Object*>();
        MO_ASSERT(obj, "no object for itemindex " << s);

        QList<Object*> mods = obj->getModulatingObjects();
        MO_DEBUG(mods.size() << " mods for '" << obj->idName() << "'");
        for (auto m : mods)
        {
            auto i = indexMap_.find(m->idName());
            if (i == indexMap_.end())
            {
                MO_WARNING("modulator '" << m->idName() << "' not found in indexMap_");
                continue;
            }

            modPaths_.push_back(ModPath_(i.value(), s));
        }

        // through childs
        //getModulations_(s);
    }
    */
}

} // namespace GUI
} // namespace MO
