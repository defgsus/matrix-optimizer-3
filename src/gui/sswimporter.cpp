/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2015</p>
*/

#include <QLayout>
#include <QTreeView>
#include <QToolButton>
#include <QMessageBox>
#include <QLabel>
#include <QTextBrowser>

#include "sswimporter.h"
#include "model/jsontreemodel.h"
#include "object/object.h"
#include "object/util/objecteditor.h"
#include "object/scene.h"
#include "io/sswproject.h"
#include "io/settings.h"
#include "io/files.h"
#include "io/error.h"

namespace MO {
namespace GUI {

struct SswImporter::Private
{
    Private(SswImporter * w)
        : widget        (w)
        , ssw           (0)
        , model         (0)
        , tree          (0)
        , rootObject    (0)
    {

    }

    void createWidgets();
    void loadProject(const QString& fn);
    void createObjects();

    SswImporter * widget;
    SswProject * ssw;
    JsonTreeModel * model;
    QTreeView * tree;
    QTextBrowser * infoBrowser;
    Object * rootObject;
};

SswImporter::SswImporter(QWidget *parent)
    : QDialog       (parent)
    , p_            (new Private(this))
{
    setObjectName("_SswImporter");
    setWindowTitle(tr("SSW project import"));
    settings()->restoreGeometry(this);

    p_->createWidgets();
}

SswImporter::~SswImporter()
{
    settings()->storeGeometry(this);
    delete p_;
}

void SswImporter::setRootObject(Object *o)
{
    p_->rootObject = o;
}

void SswImporter::Private::createWidgets()
{
    auto lv = new QVBoxLayout(widget);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto but = new QToolButton(widget);
            but->setText("open");
            lh->addWidget(but);
            connect(but, &QToolButton::clicked, [=]()
            {
                QString fn = IO::Files::getOpenFileName(IO::FT_SSW_PROJECT, widget);
                if (!fn.isEmpty())
                    loadProject(fn);
            });

            lh->addStretch(1);

            but = new QToolButton(widget);
            but->setText("create objects");
            lh->addWidget(but);
            connect(but, &QToolButton::clicked, [=]()
            {
                if (rootObject)
                {
                    try
                    {
                        createObjects();
                    }
                    catch (const Exception& e)
                    {
                        QMessageBox::critical(widget, tr("ssw object creation"),
                                              tr("Error!\n%1").arg(e.what()));
                    }
                }
            });

        tree = new QTreeView(widget);
        lv->addWidget(tree);

        infoBrowser = new QTextBrowser(widget);
        lv->addWidget(infoBrowser);
}

void SswImporter::Private::loadProject(const QString& fn)
{
    delete ssw;
    ssw = new SswProject();

    delete model;
    model = 0;

    try
    {
        ssw->load(fn);
        model = ssw->createTreeModel();
        tree->setModel(model);
        tree->setColumnWidth(0, 500);
        infoBrowser->setHtml(ssw->infoString());
    }
    catch (const Exception& e)
    {
        tree->setModel(0);
        infoBrowser->clear();
        delete ssw;
        ssw = 0;
        delete model;
        model = 0;
        QMessageBox::critical(widget, tr("SSW Import"),
                              tr("Import failed!\n%1").arg(e.what()));
    }
}

void SswImporter::Private::createObjects()
{
    if (!ssw || !rootObject)
        return;

    int k = 0;
    QList<Object*> objs;
    for (auto src : ssw->soundSources())
    {
        bool created;
        auto o = src->createObject(rootObject, created);
        o->setAttachedData(QPoint(ssw->soundSources().size() - k, k),
                           Object::DT_GRAPH_POS);
        if (created)
            objs << o;
        ++k;
    }

    if (!objs.isEmpty())
    {
        if (rootObject->editor())
            rootObject->editor()->addObjects(rootObject, objs);
        else if (rootObject->sceneObject())
            rootObject->sceneObject()->addObjects(rootObject, objs);
        else
            MO_ERROR("Can't add objects");
    }
}


} // namespace GUI
} // namespace MO
