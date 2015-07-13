/** @file assetbrowser.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2015</p>
*/

#include <QTreeView>
#include <QFileSystemModel>
#include <QLayout>
#include <QToolButton>
#include <QDir>
#include <QStringList>
#include <QMouseEvent>

#include "assetbrowser.h"
#include "io/settings.h"
#include "io/log.h"

namespace MO {
namespace GUI {

struct AssetBrowser::Private
{
    Private(AssetBrowser * w)
        : widget        (w)
        , fsmodel       (0)
        , curDirIndex   (0)
    {

    }

    void createWidgets();
    void updateModel();

    AssetBrowser * widget;

    QFileSystemModel * fsmodel;
    QTreeView * treeView;

    QStringList directories;
    uint curDirIndex;
};

AssetBrowser::AssetBrowser(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    setObjectName("_AssetBrowser");
    p_->createWidgets();
    p_->updateModel();

    selectDirectory(0);
}

AssetBrowser::~AssetBrowser()
{
    delete p_;
}


void AssetBrowser::Private::createWidgets()
{
    auto lv0 = new QVBoxLayout(widget);

        auto lh = new QHBoxLayout();
        lv0->addLayout(lh);

        // directory short-cut buttons
        for (int i=0; i<10; ++i)
        {
            auto b = new QToolButton(widget);
            b->setText(QString::number(i));
            b->setCheckable(true);
            lh->addWidget(b);
            connect(b, &QToolButton::clicked, [=]()
            {
                widget->selectDirectory(i);
            });

            QString key = QString("AssetBrowser/Directory/%1").arg(i);
            directories << (settings()->contains(key)
                            ? settings()->getValue(key).toString()
                            : "./data");
        }
        lh->addStretch(1);

        auto lv = new QVBoxLayout();
        lv0->addLayout(lv);

            treeView = new QTreeView(widget);
            lv->addWidget(treeView);

            treeView->setDragEnabled(true);
            treeView->setDragDropMode(QTreeView::DragOnly);


}

void AssetBrowser::Private::updateModel()
{
    if (!fsmodel)
        fsmodel = new QFileSystemModel(widget);

    fsmodel->setRootPath(QDir::currentPath());

    treeView->setModel(fsmodel);
    treeView->setExpandsOnDoubleClick(false);
    treeView->setHeaderHidden(true);
    for (int i=1; i < 5; ++i)
        treeView->setColumnHidden(i, true);

    treeView->setRootIndex(fsmodel->index(QDir::currentPath()));
}

void AssetBrowser::selectDirectory(uint index)
{
    index = std::min(uint(p_->directories.size()), index);
    p_->curDirIndex = index;
    p_->treeView->setRootIndex(p_->fsmodel->index(p_->directories[index]));
}

void AssetBrowser::setDirectory(uint index, const QString &dir)
{
    if (index >= uint(p_->directories.size()))
        return;

    p_->directories[index] = dir;
    p_->treeView->setRootIndex(p_->fsmodel->index(p_->directories[index]));
}

void AssetBrowser::mouseDoubleClickEvent(QMouseEvent * e)
{
    auto lpos = p_->treeView->mapFrom(this, e->pos());
    auto idx = p_->treeView->indexAt(lpos);

    MO_PRINT(idx);

    if (idx.isValid())
    {
        setDirectory(p_->curDirIndex, p_->fsmodel->filePath(idx));
        e->accept();
    }
}


} // namespace GUI
} // namespace MO
