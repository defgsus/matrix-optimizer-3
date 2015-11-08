/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 11/8/2015</p>
*/

#include <QLayout>


#include "csgdialog.h"
#include "gui/widget/csgeditwidget.h"
#include "io/settings.h"

namespace MO {
namespace GUI {

struct CsgDialog::Private
{
    Private(CsgDialog * win)
        : win       (win)
    { }

    void createWidgets();

    CsgDialog * win;
    CsgEditWidget * csgEdit;
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

void CsgDialog::Private::createWidgets()
{
    auto lh = new QHBoxLayout(win);

        csgEdit = new CsgEditWidget(win);
        lh->addWidget(csgEdit);
}



} // namespace GUI
} // namespace MO
