/** @file iconbar.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 06.03.2015</p>
*/

#include <QLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

#include "iconbar.h"

namespace MO {
namespace GUI {

namespace {

/** Wrapper for an icon with drag event */
class BarIcon : public QLabel
{
public:
    BarIcon(const QString& mimeType,
            const QByteArray& mimeData,
            QWidget * p)
        : QLabel(p), mimeType(mimeType), mimeData(mimeData) { }

protected:

    QString mimeType;
    QByteArray mimeData;

    void mouseMoveEvent(QMouseEvent * e)
    {
        if (e->buttons() & Qt::LeftButton)
        {
            auto drag = new QDrag(this);
            auto data = new QMimeData();
            data->setData(mimeType, mimeData);
            drag->setMimeData(data);
            drag->setPixmap(*pixmap());
            drag->exec(Qt::CopyAction);
        }
    }
};


} // namespace







IconBar::IconBar(QWidget *parent) :
    QTabWidget(parent)
{
}

QWidget * IconBar::getGroupWidget(const QString& group)
{
    auto i = p_widgets_.find(group);
    if (i != p_widgets_.end())
        return i.value();

    auto w = new QWidget();
    new QHBoxLayout(w);

    addTab(w, group);
    p_widgets_.insert(group, w);

    return w;
}

void IconBar::addIcon(
                const QIcon& icon,
                const QString& toolTip,
                const QString& mimeType,
                const QByteArray& mimeData,
                const QString& group)
{
    auto w = getGroupWidget(group);

    auto l = new BarIcon(mimeType, mimeData, w);
    l->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    l->setPixmap(icon.pixmap(QSize(32,32)));
    l->setToolTip(toolTip);
    l->setStatusTip(toolTip);
    w->layout()->addWidget(l);

}

void IconBar::addStretch(const QString &group, int stretch)
{
    auto w = getGroupWidget(group);
    static_cast<QHBoxLayout*>(w->layout())->addStretch(stretch);
}

void IconBar::finish()
{
    for (auto i = p_widgets_.begin(); i != p_widgets_.end(); ++i)
        addStretch(i.key());
}


} // namespace GUI
} // namespace MO
