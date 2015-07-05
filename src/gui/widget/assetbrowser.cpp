/** @file assetbrowser.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2015</p>
*/

#include <QTreeView>
#include <QFileSystemModel>
#include <QLayout>
#include <QDir>


#include "assetbrowser.h"

namespace MO {
namespace GUI {

struct AssetBrowser::Private
{
    Private(AssetBrowser * w)
        : widget        (w)
        , fsmodel       (0)
    {

    }

    void createWidgets();
    void updateModel();

    AssetBrowser * widget;

    QFileSystemModel * fsmodel;
    QTreeView * treeView;
};

AssetBrowser::AssetBrowser(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    p_->createWidgets();
    p_->updateModel();
}

AssetBrowser::~AssetBrowser()
{
    delete p_;
}


void AssetBrowser::Private::createWidgets()
{
    auto lv0 = new QVBoxLayout(widget);

        auto lv = new QVBoxLayout();
        lv0->addLayout(lv);

            treeView = new QTreeView(widget);
            lv->addWidget(treeView);


}

void AssetBrowser::Private::updateModel()
{
    if (!fsmodel)
        fsmodel = new QFileSystemModel(widget);

    fsmodel->setRootPath(QDir::currentPath());

    treeView->setModel(fsmodel);
}


} // namespace GUI
} // namespace MO
