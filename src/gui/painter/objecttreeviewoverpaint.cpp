/** @file objecttreeviewoverpaint.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/18/2014</p>
*/

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

    penPath_ = QPen(QColor(0,0,0,50));
    penPathSel_ = QPen(QColor(0,90,90,100));
    penPathSel_.setWidth(2);
}

void ObjectTreeViewOverpaint::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);


    getIndexMap_();
    getModulations_();
    updateIndexMapVisual_();

    const QModelIndex sel = view_->currentIndex();
    for (const ModPath_& m : modPaths_)
    {
        if (m.from == sel || m.to == sel)
            p.setPen(penPathSel_);
        else
            p.setPen(penPath_);

        p.drawPath(m.path);
    }
}

void ObjectTreeViewOverpaint::updateAll()
{
    update();
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

        QModelIndex idx = view_->getVisibleIndexForObject(obj);

        if (idx.isValid())
            indexMap_.insert(obj->idName(), idx);

        // go through childs
        getIndexMap_(obj);
    }

}

void ObjectTreeViewOverpaint::updateIndexMapVisual_()
{
    for (ModPath_ & m : modPaths_)
    {
        QRect rect = view_->visualRect(m.from);
        QAbstractItemDelegate * d = view_->itemDelegate(m.from);
        const QPoint pfrom = QPoint(rect.left() + 6
                                     + d->sizeHint(view_->viewOptions(), m.from).width(),
                                     rect.top() + rect.height() / 2 - 1);

        rect = view_->visualRect(m.to);
        d = view_->itemDelegate(m.to);
        const QPoint pto = QPoint(rect.left() +
                                   + d->sizeHint(view_->viewOptions(), m.to).width(),
                                   rect.top() + rect.height() / 2 + 5);

        // control-point offset
        const int
                coff = std::max(50, std::min(300,
                                        std::abs(pto.y() - pfrom.y()) / 3
                                  )),
                yhalf = (pto.y() - pfrom.y()) / 4,
                xoff = std::max(pfrom.x(), pto.x()) + coff;
        // path
        QPainterPath path(pfrom);
        path.cubicTo(xoff        , pfrom.y() + yhalf,
                     xoff        , pto.y()   - yhalf,
                     pto.x() + 5 , pto.y() );
        // arrow
        QPolygon poly;
        poly << QPoint(pto.x(),     pto.y())
             << QPoint(pto.x() + 5, pto.y() - 4)
             << QPoint(pto.x() + 5, pto.y() + 4)
             << QPoint(pto.x(),     pto.y());
        path.addPolygon(poly);

        m.path = path;
    }
}

void ObjectTreeViewOverpaint::getModulations_()
{
    modPaths_.clear();
    if (view_->sceneObject())
        getModulations_(view_->sceneObject());
}

void ObjectTreeViewOverpaint::getModulations_(Object * parent)
{
    for (auto obj : parent->childObjects())
    {
        // XXX don't display track input modulations
        // because *currently* tracks don't have inputs
        // except from sequences that are on tracks
        if (!obj->isTrack())
        {
            auto i = indexMap_.find(obj->idName());
            if (i == indexMap_.end())
            {
                MO_WARNING("object '" << obj->idName() << "' not found in indexMap_");
                continue;
            }
            const QModelIndex idx = i.value();

            QList<Object*> mods = obj->getModulatingObjects();

            for (auto m : mods)
            {
                // don't display sequence modulators
                // because they are part of a track-modulator
                if (m->isSequence())
                    continue;

                i = indexMap_.find(m->idName());
                if (i == indexMap_.end())
                {
                    MO_WARNING("modulator '" << m->idName() << "' not found in indexMap_");
                    continue;
                }

                // don't display self-references
                if (i.value() != idx)
                    modPaths_.push_back(ModPath_(i.value(), idx));
            }
        }

        // go through childs
        getModulations_(obj);
    }

}

} // namespace GUI
} // namespace MO
