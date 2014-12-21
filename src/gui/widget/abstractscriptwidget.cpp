/** @file abstractscriptwidget.cpp

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 21.12.2014</p>
*/

#include <QPlainTextEdit>
#include <QLayout>
#include <QTimer>
#include <QStyle>
#include <QListWidget>

#include "abstractscriptwidget.h"

namespace MO {
namespace GUI {


class AbstractScriptWidget::PrivateSW
{
public:

    PrivateSW(AbstractScriptWidget * widget)
        : widget    (widget),
          isValid   (false)
    { }

    void createWidgets();
    void updateEditorColor();
    void updateErrorWidget();

    void onTextChanged();

    struct Error
    {
        Error() : line(0) { }
        Error(int l, const QString& t) : line(l), text(t) { }

        int line;
        QString text;
    };

    AbstractScriptWidget * widget;

    QPlainTextEdit * editor;
    QListWidget * errorList;
    QTimer * timer;

    bool isValid;

    QString curText;

    QList<Error> errors;
};


AbstractScriptWidget::AbstractScriptWidget(QWidget *parent)
    : QWidget       (parent),
      p_sw_         (new PrivateSW(this))
{
    p_sw_->createWidgets();
}

AbstractScriptWidget::~AbstractScriptWidget()
{
    delete p_sw_;
}

bool AbstractScriptWidget::isScriptValid() const
{
    return p_sw_->isValid;
}

const QString AbstractScriptWidget::scriptText() const
{
    return p_sw_->curText;
}

void AbstractScriptWidget::setScriptText(const QString & t)
{
    p_sw_->isValid = false;
    p_sw_->curText = t;
    p_sw_->editor->setPlainText(t);
}


void AbstractScriptWidget::PrivateSW::createWidgets()
{
    auto lv = new QVBoxLayout(widget);

        // --- editor ---

        editor = new QPlainTextEdit(widget);
        lv->addWidget(editor, 10);
        connect(editor, &QPlainTextEdit::textChanged, [=]() { timer->start(); } );

        // --- setup font ---

        // XXX does not work for Mac
        QFont f("Monospace");
        f.setStyleHint(QFont::Monospace);
        editor->setFont(f);
        editor->setTabStopWidth(QFontMetrics(f).width("    "));


        errorList = new QListWidget(widget);
        errorList->setVisible(false);
        lv->addWidget(errorList);

        /*
        auto but = new QPushButton(tr("run"), widget);
        lv->addWidget(but);
        but->setShortcut(Qt::CTRL + Qt::Key_R);
        connect(but, &QPushButton::pressed, [=]() { run(); } );
        */

        timer = new QTimer(widget);
        timer->setSingleShot(true);
        timer->setInterval(250);
        connect(timer, &QTimer::timeout, [=]() { onTextChanged(); } );
}


void AbstractScriptWidget::PrivateSW::onTextChanged()
{
    curText = editor->toPlainText();

    errors.clear();
    isValid = widget->compile();

    updateEditorColor();
    updateErrorWidget();
}

void AbstractScriptWidget::PrivateSW::updateEditorColor()
{
    QPalette p(widget->style()->standardPalette());

    if (!isValid)
    {
        p.setColor(QPalette::Base, QColor(100,50,50));
    }

    editor->setPalette(p);
}

void AbstractScriptWidget::PrivateSW::updateErrorWidget()
{
    if (errors.isEmpty())
    {
        errorList->setVisible(false);
        return;
    }

    errorList->clear();
    errorList->setVisible(true);

    // build list items
    for (const Error& e : errors)
    {
        auto item = new QListWidgetItem(errorList);
        item->setText(QString(tr("error line %1: %2")
                              .arg(e.line)
                              .arg(e.text)));
        item->setData(Qt::UserRole + 1, e.line);
        errorList->addItem(item);
    }
}


void AbstractScriptWidget::addScriptError(int line, const QString &text)
{
    p_sw_->errors << PrivateSW::Error(line, text);
}


} // namespace GUI
} // namespace MO
