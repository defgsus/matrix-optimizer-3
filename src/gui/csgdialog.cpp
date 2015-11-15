/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#include <QLayout>
#include <QCloseEvent>
#include <QToolButton>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileInfo>

#include "csgdialog.h"
#include "gui/widget/csgeditwidget.h"
#include "gui/widget/csgrenderwidget.h"
#include "gui/propertiesscrollview.h"
#include "math/csgbase.h"
#include "types/properties.h"
#include "io/settings.h"
#include "io/files.h"

namespace MO {
namespace GUI {

struct CsgDialog::Private
{
    Private(CsgDialog * win)
        : win           (win)
        , isModified    (false)
        , closeRequest  (false)
    { }

    void createWidgets();
    void createMenu();
    void setViewDirection(Basic3DWidget::ViewDirection);
    void updateTitle();

    CsgDialog * win;
    CsgEditWidget * csgEdit;
    CsgRenderWidget * renderWidget;
    PropertiesScrollView * renderPropView;
    QString curFilename;
    bool isModified;
    bool closeRequest;
};

CsgDialog::CsgDialog(QWidget *parent)
    : QMainWindow   (parent)
    , p_        (new Private(this))
{
    setObjectName("_CsgDialog");
    settings()->restoreGeometry(this);

    p_->createWidgets();
    p_->createMenu();

    p_->updateTitle();
}

CsgDialog::~CsgDialog()
{
    settings()->storeGeometry(this);
    delete p_;
}

void CsgDialog::closeEvent(QCloseEvent*e)
{
    if (p_->renderWidget->isGlInitialized())
    {
        p_->renderWidget->shutDownGL();
        p_->closeRequest = true;
        e->ignore();
    }
    else QMainWindow::closeEvent(e);
}

void CsgDialog::Private::createWidgets()
{       
    win->setCentralWidget(new QWidget(win));
    auto lh = new QHBoxLayout(win->centralWidget());

        csgEdit = new CsgEditWidget(win);
        lh->addWidget(csgEdit);
        connect(csgEdit, &CsgEditWidget::changed, [this]()
        {
            renderWidget->setRootObject(csgEdit->rootObject());
            bool isMod = isModified;
            isModified = true;
            if (!isMod)
                updateTitle();
        });

        auto lv = new QVBoxLayout();
        lv->setMargin(0);
        lh->addLayout(lv, 2);

            renderWidget = new CsgRenderWidget(win);
            renderWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            lv->addWidget(renderWidget);
            connect (renderWidget, &CsgRenderWidget::glReleased, [=]()
            {
                if (closeRequest)
                    win->close();
            });

            renderWidget->setRootObject(csgEdit->rootObject());

            auto lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                auto tbut = new QToolButton(win);
                lh2->addWidget(tbut);
                tbut->setStatusTip(tr("front"));
                tbut->setIcon(QIcon(":/icon/view_front.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_FRONT); });

                tbut = new QToolButton(win);
                lh2->addWidget(tbut);
                tbut->setStatusTip(tr("back"));
                tbut->setIcon(QIcon(":/icon/view_back.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_BACK); });

                tbut = new QToolButton(win);
                lh2->addWidget(tbut);
                tbut->setStatusTip(tr("left"));
                tbut->setIcon(QIcon(":/icon/view_left.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_LEFT); });

                tbut = new QToolButton(win);
                lh2->addWidget(tbut);
                tbut->setStatusTip(tr("right"));
                tbut->setIcon(QIcon(":/icon/view_right.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_RIGHT); });

                tbut = new QToolButton(win);
                lh2->addWidget(tbut);
                tbut->setStatusTip(tr("top"));
                tbut->setIcon(QIcon(":/icon/view_top.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_TOP); });

                tbut = new QToolButton(win);
                lh2->addWidget(tbut);
                tbut->setStatusTip(tr("bottom"));
                tbut->setIcon(QIcon(":/icon/view_bottom.png"));
                connect(tbut, &QToolButton::clicked,
                        [=]{ setViewDirection(Basic3DWidget::VD_BOTTOM); });

                lh2->addStretch();

            renderPropView = new PropertiesScrollView(win);
            renderPropView->setProperties(renderWidget->shaderProperties());
            lv->addWidget(renderPropView);
            connect(renderPropView, &PropertiesScrollView::propertyChanged, [this]()
            {
                renderWidget->setShaderProperties(renderPropView->properties());
            });
}

void CsgDialog::Private::setViewDirection(Basic3DWidget::ViewDirection d)
{
    renderWidget->viewSet(d, 5.);
}

void CsgDialog::Private::createMenu()
{
    win->setMenuBar( new QMenuBar(win) );

    auto menu = win->menuBar()->addMenu(tr("File"));
    auto a = menu->addAction(tr("Load"));
    a->setShortcut(Qt::CTRL + Qt::Key_O);
    connect(a, SIGNAL(triggered(bool)), win, SLOT(loadFile()));
    a = menu->addAction(tr("Save"));
    a->setShortcut(Qt::CTRL + Qt::Key_S);
    connect(a, SIGNAL(triggered(bool)), win, SLOT(saveFile()));
    a = menu->addAction(tr("Save as ..."));
    a->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_S);
    connect(a, SIGNAL(triggered(bool)), win, SLOT(saveFileAs()));
}

bool CsgDialog::isSaveToChange()
{
    if (!p_->isModified)
        return true;

    int a =
        QMessageBox::question(this, tr("keep changes"),
                              tr("The current tree has unsaved changes, "
                                 "do you want to save it?"),
                              tr("Save as"), tr("Discard"), tr("Cancel"), 0, 2);
    if (a == 0)
    {
        if (saveFileAs())
            return true;
    }
    if (a == 1)
        return true;

    return false;
}

void CsgDialog::loadFile()
{
    if (!isSaveToChange())
        return;

    auto fn = IO::Files::getOpenFileName(IO::FT_CSG_XML, this);
    if (fn.isEmpty())
        return;

    loadFile(fn);
}

bool CsgDialog::saveFile()
{
    if (p_->curFilename.isEmpty())
        return saveFileAs();
    else
        return saveFile(p_->curFilename);
}

bool CsgDialog::saveFileAs()
{
    auto fn = IO::Files::getSaveFileName(IO::FT_CSG_XML, this);
    if (fn.isEmpty())
        return false;

    return saveFile(fn);
}

void CsgDialog::loadFile(const QString &fn)
{
    try
    {
        auto base = CsgBase::loadXml(fn);
        auto root = dynamic_cast<CsgRoot*>(base);
        if (!root)
            MO_IO_ERROR(READ, "No root node in csg tree");

        setRootObject(root);
        p_->curFilename = fn;
        p_->isModified = false;
        p_->updateTitle();
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("load"),
                              tr("Could not load '%1'\n%2")
                              .arg(fn).arg(e.what()));
    }
}

bool CsgDialog::saveFile(const QString &fn)
{
    try
    {
        p_->csgEdit->rootObject()->saveXml(fn);
        p_->curFilename = fn;
        p_->isModified = false;
        p_->updateTitle();
        return true;
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("save"),
                              tr("Could not save '%1'\n%2")
                              .arg(fn).arg(e.what()));
    }
    return false;
}

void CsgDialog::setRootObject(CsgRoot* root)
{
    p_->csgEdit->setRootObject(root);
    p_->renderWidget->setRootObject(root);
}

void CsgDialog::Private::updateTitle()
{
    auto s = tr("CSG Object Editor");
    if (!curFilename.isEmpty())
        s += " (" + QFileInfo(curFilename).fileName() + ")";
    if (isModified)
        s += tr(" [modified]");
    win->setWindowTitle(s);
}

} // namespace GUI
} // namespace MO
