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
#include "widget/helptextbrowser.h"
#include "io/log.h"

namespace MO {
namespace GUI {


HelpDialog::HelpDialog(QWidget *parent) :
    QDialog     (parent),
    historyPos_ (0)
{
    setMinimumSize(640,480);

    createWidgets_();

    setDocument_(getHelpDocument("intro"));
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
            aBack_->setText("<");

            toolbar->addAction( aForward_ = new QAction(toolbar) );
            aForward_->setShortcut(Qt::Key_Right);
            connect(aForward_, SIGNAL(triggered()), this, SLOT(forward()));
            aForward_->setText(">");

        // --- browser ---

        browser_ = new HelpTextBrowser(this);
        lv->addWidget(browser_);

        connect(browser_, SIGNAL(anchorClicked(QUrl)),
                this, SLOT(onAnchor_(QUrl)));
}

void HelpDialog::updateActions_()
{
    aBack_->setEnabled(historyPos_ > 0);
    aForward_->setEnabled(historyPos_ < history_.size()-1);
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
    //QString doc = s.readAll();

    //doc.replace("src=\"", "src=\":/image/");

    //return doc;
}

void HelpDialog::onAnchor_(const QUrl & url)
{
//    MO_DEBUG(url.url());
    const QString doc = getHelpDocument(url.url());
    setDocument_(doc);
}

void HelpDialog::setDocument_(const QString & doc)
{
    browser_->setHtml(doc);
    history_.append(doc);
    historyPos_ = history_.size()-1;

    updateActions_();
}

void HelpDialog::backward()
{
    if (historyPos_ == 0 || historyPos_ >= history_.size())
        return;

    historyPos_--;
    browser_->setHtml(history_[historyPos_]);

    updateActions_();
}

void HelpDialog::forward()
{
    if (historyPos_ >= history_.size()-1)
        return;

    historyPos_++;
    browser_->setHtml(history_[historyPos_]);

    updateActions_();
}

} // namespace GUI
} // namespace MO
