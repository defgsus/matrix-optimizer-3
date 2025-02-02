/** @file helpdialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#include <QLayout>
#include <QToolButton>
#include <QTextBrowser>
#include <QToolBar>
#include <QAction>

#include "HelpDialog.h"
#include "widget/HelpTextBrowser.h"
#include "io/Application.h"
#include "io/Settings.h"

namespace MO {
namespace GUI {


HelpDialog::HelpDialog(QWidget *parent)
    : HelpDialog    ("index", parent)
{
}

HelpDialog::HelpDialog(const QString &url, QWidget *parent)
    : QDialog       (parent)
{
    setObjectName("_HelpDialog");

    setMinimumSize(640,480);

    settings()->restoreGeometry(this);

    createWidgets_();

    browser_->setSource(QUrl(url));
}

HelpDialog::~HelpDialog()
{
    settings()->storeGeometry(this);
}

void HelpDialog::run(const QString &url)
{
    auto help = new HelpDialog(url, application()->mainWindow());
    help->setAttribute(Qt::WA_DeleteOnClose);
    help->show();
}

void HelpDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        // navigation

        auto toolbar = new QToolBar(this);
        lv->addWidget(toolbar);

            auto aBack = new QAction(toolbar);
            toolbar->addAction( aBack );
            aBack->setShortcut(Qt::Key_Left);
            aBack->setText("<");
            aBack->setEnabled(false);

            auto aForward = new QAction(toolbar);
            toolbar->addAction( aForward );
            aForward->setShortcut(Qt::Key_Right);
            aForward->setText(">");
            aForward->setEnabled(false);

        // --- browser ---

        browser_ = new HelpTextBrowser(this);
        lv->addWidget(browser_);

        connect(browser_, SIGNAL(backwardAvailable(bool)),
                aBack, SLOT(setEnabled(bool)));
        connect(browser_, SIGNAL(forwardAvailable(bool)),
                aForward, SLOT(setEnabled(bool)));
        connect(aBack, SIGNAL(triggered()), browser_, SLOT(backward()));
        connect(aForward, SIGNAL(triggered()), browser_, SLOT(forward()));
}

} // namespace GUI
} // namespace MO
