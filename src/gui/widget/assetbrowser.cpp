/** @file assetbrowser.cpp

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/5/2015</p>
*/
#include <QDebug>
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
    void pushDir();
    void updateButtons();
    void applyFilter(const QString&);

    AssetBrowser * widget;

    QFileSystemModel * fsmodel;
    AssetTreeView * treeView;
    QVector<QToolButton*> dirButtons;
    QLineEdit * filterEdit;
    QLabel * dirLabel;
    QToolButton * butBack, * butForward;

    QStringList directories;
    uint curDirIndex;

    struct History
    {
        History() : index(0) { }
        QVector<QString> dirs;
        int index;
    };
    QVector<History> history;
};

AssetBrowser::AssetBrowser(QWidget *parent)
    : QWidget       (parent)
    , p_            (new Private(this))
{
    setObjectName("_AssetBrowser");
    p_->createWidgets();
    p_->updateModel();

    p_->history.resize(p_->directories.size());
    for (int i=0; i<p_->directories.size(); ++i)
        p_->history[i].dirs << p_->directories[i];

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

            // --- navigation ---

            dirLabel = new QLabel(widget);
            dirLabel->setToolTip(tr("Current directory"));
            lh->addWidget(dirLabel);

            // directory-up button
            auto but = new QToolButton(widget);
            but->setIcon(QIcon(":/icon/dir_up.png"));
            but->setToolTip(tr("Go up one directory"));
            connect(but, SIGNAL(clicked(bool)), widget, SLOT(goUp()));
            lh->addWidget(but);

            // backward button
            but = butBack = new QToolButton(widget);
            //but->setIcon(QIcon(":/icon/dir_up.png"));
            but->setText("<");
            but->setToolTip(tr("Go back to previous directory in history"));
            connect(but, SIGNAL(clicked(bool)), widget, SLOT(goBack()));
            lh->addWidget(but);

            // forward button
            but = butForward = new QToolButton(widget);
            //but->setIcon(QIcon(":/icon/dir_up.png"));
            but->setText(">");
            but->setToolTip(tr("Go to next directory in history"));
            connect(but, SIGNAL(clicked(bool)), widget, SLOT(goForward()));
            lh->addWidget(but);

            // --- filter edit ---

        lh = new QHBoxLayout();
        lv0->addLayout(lh);

            lh->addWidget(new QLabel(tr("filter"), widget));

            filterEdit = new QLineEdit(widget);
            filterEdit->setStatusTip(tr("Enter any letters/numbers to filter the displayed files; "
                                        "separate different filters by comma; "
                                        "use wildcard symbols '*' and '?'"));
            lh->addWidget(filterEdit);
            connect(filterEdit, &QLineEdit::textChanged, [=](const QString& f) { applyFilter(f); });

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
    p_->dirLabel->setText(p_->directories[p_->curDirIndex]);

    // store current index in settings
    settings()->setValue("AssetBrowser/curDirIndex", p_->curDirIndex);

    // pull filter from settings
    const QString key = QString("AssetBrowser/Filter/%1").arg(index);
    setFilter(settings()->getValue(key, "").toString());

    p_->updateButtons();

    /** @todo AssetBrowser: store/restore expanded-state of directories */
}

void AssetBrowser::setDirectory(uint index, const QString &dir, bool updateHistory)
{
    if (index >= uint(p_->directories.size()))
        return;

    // store runtime info
    p_->directories[index] = dir;
    p_->dirButtons[index]->setToolTip(dir);
    p_->dirButtons[index]->setStatusTip(tr("Click to show %1").arg(dir));
    // update label
    p_->dirLabel->setText(dir);
    // point file model there
    p_->treeView->setRootIndex(p_->fsmodel->index(dir));
    // store in app settings
    const QString key = QString("AssetBrowser/Directory/%1").arg(index);
    settings()->setValue(key, dir);

    // update history
    if (updateHistory)
        p_->pushDir();
}

void AssetBrowser::Private::pushDir()
{
    if (curDirIndex >= uint(history.size()))
        return;

    History& h = history[curDirIndex];
    h.dirs.resize(h.index+1);
    h.dirs << directories[curDirIndex];
    ++h.index;

    updateButtons();
}

void AssetBrowser::Private::updateButtons()
{
    if (curDirIndex >= uint(history.size()))
        return;

    qInfo() << history[curDirIndex].dirs << history[curDirIndex].index;
    butBack->setEnabled(history[curDirIndex].index > 0);
    butForward->setEnabled(history[curDirIndex].index+1
                           < history[curDirIndex].dirs.size());
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
    if (p_->curDirIndex >= (uint)p_->directories.size())
        return QString();
    return QDir(p_->directories[p_->curDirIndex]).canonicalPath();
}

void AssetBrowser::goUp()
{
    QDir dir(currentDirectory());
    if (dir.cdUp())
    {
        setDirectory(p_->curDirIndex, dir.canonicalPath());
    }
}

void AssetBrowser::goBack()
{
    Private::History& h = p_->history[p_->curDirIndex];
    if (h.index > 0)
    {
        --h.index;
        if (h.index < h.dirs.size())
        {
            setDirectory(p_->curDirIndex, h.dirs[h.index], false);
            p_->updateButtons();
        }
    }
}

void AssetBrowser::goForward()
{
    Private::History& h = p_->history[p_->curDirIndex];
    if (h.index+1 < h.dirs.size())
    {
        ++h.index;
        setDirectory(p_->curDirIndex, h.dirs[h.index], false);
        p_->updateButtons();
    }
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
