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
#include <QTableWidget>

#include "sswimporter.h"
#include "model/jsontreemodel.h"
#include "object/object.h"
#include "object/util/objecteditor.h"
#include "object/scene.h"
#include "gui/propertiesscrollview.h"
#include "types/properties.h"
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
    void updateTableWidget();
    void createObjects();

    SswImporter * widget;
    SswProject * ssw;
    JsonTreeModel * model;
    QTreeView * tree;
    //QTextBrowser * infoBrowser;
    QTableWidget * table;
    PropertiesScrollView * propView;
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
    auto lh0 = new QHBoxLayout(widget);

        auto lv = new QVBoxLayout();
        lh0->addLayout(lv);

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

            //infoBrowser = new QTextBrowser(widget);
            //lv->addWidget(infoBrowser);
            table = new QTableWidget(widget);
            table->setColumnCount(7);
            //table->setAlternatingRowColors(true);
            table->setHorizontalHeaderLabels(
                        QStringList()
                        << tr("label")
                        << tr("gain(dB)")
                        << tr("type")
                        << tr("pos")
                        << tr("num automations")
                        << tr("start")
                        << tr("end"));
            lv->addWidget(table);

        propView = new PropertiesScrollView(widget);
        lh0->addWidget(propView);
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
        propView->setProperties(ssw->properties());
        //infoBrowser->setHtml(ssw->infoString());
        updateTableWidget();
    }
    catch (const Exception& e)
    {
        tree->setModel(0);
        //infoBrowser->clear();
        table->clear();
        delete ssw;
        ssw = 0;
        delete model;
        model = 0;
        QMessageBox::critical(widget, tr("SSW Import"),
                              tr("Import failed!\n%1").arg(e.what()));
    }
}

void SswImporter::Private::updateTableWidget()
{
    table->clear();
    table->setRowCount(ssw->soundSources().size());

    for (SswSource * src : ssw->soundSources())
    {
        auto item = new QTableWidgetItem(src->label());
        item->setData(Qt::UserRole + 1, src->index());
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        item->setCheckState(Qt::Unchecked);
        table->setItem(src->index(), 0, item);

        item = new QTableWidgetItem(QString("%1").arg(src->gainDb()));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(src->index(), 1, item);

        item = new QTableWidgetItem(QString("%1").arg(src->typeName()));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(src->index(), 2, item);

        item = new QTableWidgetItem(QString("<%1, %2, %3>")
            .arg(src->position().x).arg(src->position().y).arg(src->position().z));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(src->index(), 3, item);

        item = new QTableWidgetItem(QString("%1").arg(src->automations().size()));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(src->index(), 4, item);

        item = new QTableWidgetItem(QString("%1").arg(src->startTime()));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(src->index(), 5, item);

        item = new QTableWidgetItem(QString("%1").arg(src->endTime()));
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        table->setItem(src->index(), 6, item);
    }
}

bool SswImporter::isEnabled(uint index) const
{
    if ((int)index >= p_->table->rowCount())
        return false;

    auto item = p_->table->item(index, 0);
    if (!item)
        return false;

    return item->checkState() == Qt::Checked;
}


void SswImporter::Private::createObjects()
{
    if (!ssw || !rootObject)
        return;

    ssw->setProperties(propView->properties());

    int k = 0;
    QList<Object*> objs;
    for (auto src : ssw->soundSources())
    {
        if (!widget->isEnabled(src->index()))
            continue;

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
