/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#include <QLayout>
#include <QCloseEvent>

#include "csgdialog.h"
#include "gui/widget/csgeditwidget.h"
#include "gui/widget/csgrenderwidget.h"
#include "gui/propertiesscrollview.h"
#include "types/properties.h"
#include "io/settings.h"

namespace MO {
namespace GUI {

struct CsgDialog::Private
{
    Private(CsgDialog * win)
        : win           (win)
        , closeRequest  (false)
    { }

    void createWidgets();

    CsgDialog * win;
    CsgEditWidget * csgEdit;
    CsgRenderWidget * renderWidget;
    PropertiesScrollView * renderPropView;
    bool closeRequest;
};

CsgDialog::CsgDialog(QWidget *parent)
    : QDialog   (parent)
    , p_        (new Private(this))
{
    setObjectName("_CsgDialog");
    setWindowTitle(tr("CSG Object Editor"));
    settings()->restoreGeometry(this);

    p_->createWidgets();
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
    else QDialog::closeEvent(e);
}

void CsgDialog::Private::createWidgets()
{
    auto lh = new QHBoxLayout(win);

        csgEdit = new CsgEditWidget(win);
        lh->addWidget(csgEdit);
        connect(csgEdit, &CsgEditWidget::changed, [this]()
        {
            renderWidget->setRootObject(csgEdit->rootObject());
        });

        auto lv = new QVBoxLayout();
        lv->setMargin(0);
        lh->addLayout(lv);

            renderWidget = new CsgRenderWidget(win);
            lv->addWidget(renderWidget);
            connect (renderWidget, &CsgRenderWidget::glReleased, [=]()
            {
                if (closeRequest)
                    win->close();
            });

            renderWidget->setRootObject(csgEdit->rootObject());

            renderPropView = new PropertiesScrollView(win);
            renderPropView->setProperties(renderWidget->shaderProperties());
            lv->addWidget(renderPropView);
            connect(renderPropView, &PropertiesScrollView::propertyChanged, [this]()
            {
                renderWidget->setShaderProperties(renderPropView->properties());
            });
}



} // namespace GUI
} // namespace MO
