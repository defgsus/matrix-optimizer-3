/** @file assetbrowser.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2015</p>
*/

#include <QTreeView>
#include <QFileSystemModel>
#include <QLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QLabel>
#include <QDir>
#include <QFileInfo>
#include <QStringList>
#include <QMouseEvent>

#include "assetbrowser.h"
#include "io/settings.h"
#include "io/log.h"

namespace MO {
namespace GUI {

namespace {

/** Subclass of QTreeView to catch double-clicks */
class AssetTreeView : public QTreeView
{
public:
    AssetTreeView(AssetBrowser* b, QWidget* w)
        : QTreeView(w), browser(b) { }

protected:
    void mouseDoubleClickEvent(QMouseEvent*);

    AssetBrowser * browser;
};

} // namespace

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
    void applyFilter(const QString&);

    AssetBrowser * widget;

    QFileSystemModel * fsmodel;
    AssetTreeView * treeView;
    QVector<QToolButton*> dirButtons;
    QLineEdit * filterEdit;

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

    selectDirectory(settings()->getValue("AssetBrowser/curDirIndex", 0).toUInt());
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
                b->setText(QString::number(i+1));
                b->setCheckable(true);
                b->setAutoExclusive(true);
                lh->addWidget(b);
                connect(b, &QToolButton::clicked, [=]()
                {
                    widget->selectDirectory(i);
                });
                dirButtons << b;

                // get directories from app settings
                const QString
                        key = QString("AssetBrowser/Directory/%1").arg(i),
                        dir = settings()->getValue(key, settings()->dataPath()).toString();
                directories << dir;
                b->setToolTip(dir);
                b->setStatusTip(tr("Click to show %1").arg(dir));
            }
            lh->addStretch(1);



        lh = new QHBoxLayout();
        lv0->addLayout(lh);

            // --- filter edit ---

            lh->addWidget(new QLabel(tr("filter"), widget));

            filterEdit = new QLineEdit(widget);
            filterEdit->setStatusTip(tr("Enter any letters/numbers to filter the displayed files; "
                                        "separate different filters by comma; "
                                        "use wildcard symbols '*' and '?'"));
            lh->addWidget(filterEdit);
            connect(filterEdit, &QLineEdit::textChanged, [=](const QString& f) { applyFilter(f); });

            // directory-up button
            auto but = new QToolButton(widget);
            but->setIcon(QIcon(":/icon/dir_up.png"));
            but->setToolTip(tr("Go up one directory"));
            connect(but, SIGNAL(clicked(bool)), widget, SLOT(goUp()));
            lh->addWidget(but);

        // --- tree view ---

        treeView = new AssetTreeView(widget, widget);
        lv0->addWidget(treeView);

        treeView->setDragEnabled(true);
        treeView->setDragDropMode(QTreeView::DragOnly);
        treeView->setStatusTip(
                    tr("Drag a file into the scene; "
                       "double-click directory to set as root directory; "
                       "double-click scene file to load"));


}

void AssetBrowser::Private::updateModel()
{
    if (!fsmodel)
        fsmodel = new QFileSystemModel(widget);

    fsmodel->setRootPath(QDir::currentPath());
    fsmodel->setNameFilterDisables(false);

    treeView->setModel(fsmodel);
    treeView->setExpandsOnDoubleClick(false);
    treeView->setHeaderHidden(true);
    // hide the file-info columns
    for (int i=1; i < 5; ++i)
        treeView->setColumnHidden(i, true);

    treeView->setRootIndex(fsmodel->index(QDir::currentPath()));
}

void AssetBrowser::selectDirectory(uint index)
{
    index = std::min(uint(p_->directories.size()), index);

    p_->curDirIndex = index;
    p_->dirButtons[index]->setChecked(true);

    p_->treeView->setRootIndex(p_->fsmodel->index(p_->directories[index]));

    // store current index in settings
    settings()->setValue("AssetBrowser/curDirIndex", p_->curDirIndex);

    // pull filter from settings
    const QString key = QString("AssetBrowser/Filter/%1").arg(index);
    setFilter(settings()->getValue(key, "").toString());

    /** @todo AssetBrowser: store/restore expanded-state of directories */
}

void AssetBrowser::setDirectory(uint index, const QString &dir)
{
    if (index >= uint(p_->directories.size()))
        return;

    // store runtime info
    p_->directories[index] = dir;
    p_->dirButtons[index]->setToolTip(dir);
    p_->dirButtons[index]->setStatusTip(tr("Click to show %1").arg(dir));
    // point file model there
    p_->treeView->setRootIndex(p_->fsmodel->index(dir));
    // store in app settings
    const QString key = QString("AssetBrowser/Directory/%1").arg(index);
    settings()->setValue(key, dir);
}

void AssetBrowser::setFilter(const QString &f)
{
    p_->filterEdit->setText(f);
}

void AssetBrowser::Private::applyFilter(const QString& f)
{
    // split by comma
    QStringList list = f.split(',', QString::SkipEmptyParts);
    // attach '*' if not there
    for (auto & s : list)
    {
        s = s.simplified();
        if (!s.contains('*'))
            s = "*" + s + "*";
    }
    // filter fs model
    fsmodel->setNameFilters(list);
    // store in settings
    const QString key = QString("AssetBrowser/Filter/%1").arg(curDirIndex);
    settings()->setValue(key, f);
}

QString AssetBrowser::currentDirectory() const
{
    return QDir(p_->directories[p_->curDirIndex]).canonicalPath();
}

void AssetBrowser::goUp()
{
    QDir dir(currentDirectory());
    if (dir.cdUp())
        setDirectory(p_->curDirIndex, dir.canonicalPath());
}

void AssetTreeView::mouseDoubleClickEvent(QMouseEvent * e)
{
    auto idx = indexAt(e->pos());
    if (idx.isValid())
    {
        browser->doubleClick(idx);
        e->accept();
        return;
    }

    QTreeView::mouseDoubleClickEvent(e);
}

void AssetBrowser::doubleClick(const QModelIndex& idx)
{
    const QString fn = p_->fsmodel->filePath(idx);
    if (fn.isEmpty())
        return;

    if (fn.endsWith("mo3"))
    {
        emit sceneSelected(fn);
        return;
    }

    // select directory
    if (QFileInfo(fn).isDir())
        setDirectory(p_->curDirIndex, fn);
}


} // namespace GUI
} // namespace MO
