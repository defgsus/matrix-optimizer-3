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

#include "sswimporter.h"
#include "model/jsontreemodel.h"
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
        , tree          (0)
        , model         (0)
    {

    }

    void createWidgets();

    SswImporter * widget;
    QTreeView * tree;
    JsonTreeModel * model;
};

SswImporter::SswImporter(QWidget *parent)
    : QDialog       (parent)
    , p_            (new Private(this))
{
    setObjectName("_SswImporter");
    settings()->restoreGeometry(this);

    p_->createWidgets();
}

SswImporter::~SswImporter()
{
    settings()->storeGeometry(this);
    delete p_;
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
                if (fn.isEmpty())
                    return;
                SswProject ssw;
                try
                {
                    ssw.load(fn);
                    delete model;
                    model = ssw.createModel();
                    tree->setModel(model);
                    tree->setColumnWidth(0, 500);
                }
                catch (const Exception& e)
                {
                    QMessageBox::critical(widget, tr("SSW Import"),
                                          tr("Import failed!\n%1").arg(e.what()));
                }
            });

            lh->addStretch(1);

        tree = new QTreeView(widget);
        lv->addWidget(tree);
}



} // namespace GUI
} // namespace MO
