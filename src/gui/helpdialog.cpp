/** @file helpdialog.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/30/2014</p>
*/

#include <QLayout>
#include <QToolButton>
#include <QTextBrowser>
#include <QTextStream>
#include <QFile>
#include <QToolBar>
#include <QAction>

#include "helpdialog.h"
#include "io/log.h"

namespace MO {
namespace GUI {


HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent)
{
    setMinimumSize(640,480);

    createWidgets_();
}

void HelpDialog::createWidgets_()
{
    auto lv = new QVBoxLayout(this);

        // navigation

        auto toolbar = new QToolBar(this);
        lv->addWidget(toolbar);

        toolbar->addAction( aBack_ = new QAction(toolbar) );
        aBack_->setShortcut(Qt::Key_Left);
        connect(aBack_, SIGNAL(triggered()), this, SLOT(backward()));

        toolbar->addAction( aForward_ = new QAction(toolbar) );
        aForward_->setShortcut(Qt::Key_Right);
        connect(aForward_, SIGNAL(triggered()), this, SLOT(forward()));
        /*
        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            auto butBack = new QToolButton(this);
            lh->addWidget(butBack);
            butBack->setText("<");
            butBack->setEnabled(false);

            auto butForward = new QToolButton(this);
            lh->addWidget(butForward);
            butForward->setText(">");
            butForward->setEnabled(false);
        */
        // browser

        browser_ = new QTextBrowser(this);
        lv->addWidget(browser_);

        browser_->setHtml(getHelpDocument("intro"));
        connect(browser_, SIGNAL(anchorClicked(QUrl)),
                this, SLOT(onAnchor_(QUrl)));
}

QString HelpDialog::getHelpDocument(const QString& name)
{
    QString fn = ":/help/"
            + (name.endsWith(".html")? name : name + ".html");

    QFile f(fn);
    if (!f.open(QFile::ReadOnly))
    {
        return tr("<h3>%1 is missing</h3>").arg(fn);
    }

    QTextStream s(&f);
    return s.readAll();
}

void HelpDialog::onAnchor_(const QUrl & url)
{
//    MO_DEBUG(url.url());
    browser_->setSource(url);
    browser_->setHtml(getHelpDocument(url.url()));
}

void HelpDialog::backward()
{

}

void HelpDialog::forward()
{

}

} // namespace GUI
} // namespace MO
