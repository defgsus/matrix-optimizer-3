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
#include <QSyntaxHighlighter>

#include "abstractscriptwidget.h"
#include "io/settings.h"
//#include "io/log.h"


namespace MO {
namespace GUI {


class AbstractScriptWidget::PrivateSW
{
public:

    PrivateSW(AbstractScriptWidget * widget)
        : widget    (widget),
          isValid   (false),
          ignoreTextChange(false)
    { }

    void createWidgets();
    void updateEditorColor();
    void updateErrorWidget();

    void onTextChanged();

    /** Returns the position for the line number */
    int posForLine(int line) const;

    struct Message
    {
        Message() : line(0) { }
        Message(int l, MessageType ty, const QString& t)
            : line(l), type(ty), text(t) { }

        int line;
        MessageType type;
        QString text;
    };

    AbstractScriptWidget * widget;

    QPlainTextEdit * editor;
    QListWidget * messageList;
    QTimer * timer;

    bool isValid, ignoreTextChange;

    QString curText;

    QList<Message> messages;
};


AbstractScriptWidget::AbstractScriptWidget(QWidget *parent)
    : QWidget       (parent),
      p_sw_         (new PrivateSW(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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

void AbstractScriptWidget::setSyntaxHighlighter(QSyntaxHighlighter * s)
{
    p_sw_->ignoreTextChange = true;

    if (s->document() != p_sw_->editor->document())
        s->setDocument( p_sw_->editor->document() );

    s->rehighlight();

    p_sw_->ignoreTextChange = false;
}

void AbstractScriptWidget::PrivateSW::createWidgets()
{
    auto lv = new QVBoxLayout(widget);
    //lv->setSizeConstraint(QLayout::SetMaximumSize);

        // --- editor ---

        editor = new QPlainTextEdit(widget);
        editor->setProperty("code", true);
        editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        // XXX
        editor->setMinimumHeight(600);
        lv->addWidget(editor, 10);
        connect(editor, &QPlainTextEdit::textChanged, [=]() { if (!ignoreTextChange) timer->start(); } );
        //editor->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

        // --- setup font ---

        // XXX does not work for Mac
        QFont f("Monospace");
        f.setStyleHint(QFont::Monospace);
        editor->setFont(f);
        editor->setTabStopWidth(QFontMetrics(editor->font()).width("    "));


        // --- bottom display ----

        messageList = new QListWidget(widget);
        messageList->setVisible(false);
        messageList->setSizeAdjustPolicy(QListWidget::AdjustToContents);
        lv->addWidget(messageList);
        connect(messageList, &QListWidget::itemClicked, [=](QListWidgetItem*item)
        {
            int line = item->data(Qt::UserRole + 1).toInt();
            QTextCursor curs(editor->textCursor());
            curs.setPosition(posForLine(line));
            curs.select(QTextCursor::LineUnderCursor);
            editor->setTextCursor(curs);
        });

        /*
        auto but = new QPushButton(tr("run"), widget);
        lv->addWidget(but);
        but->setShortcut(Qt::CTRL + Qt::Key_R);
        connect(but, &QPushButton::pressed, [=]() { run(); } );
        */

        timer = new QTimer(widget);
        timer->setSingleShot(true);
        timer->setInterval(600);
        connect(timer, &QTimer::timeout, [=]() { onTextChanged(); } );
}


void AbstractScriptWidget::PrivateSW::onTextChanged()
{
    curText = editor->toPlainText();

    messages.clear();
    isValid = widget->compile();

    updateEditorColor();
    updateErrorWidget();

    if (isValid)
        emit widget->scriptTextChanged();
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
    if (messages.isEmpty())
    {
        messageList->setVisible(false);
        return;
    }

    messageList->clear();
    messageList->setVisible(true);

    // build list items
    for (const Message& e : messages)
    {
        // construct displayed text
        QString n;
        if (e.type == M_INFO)
            n = tr("info");
        if (e.type == M_WARNING)
            n = tr("warning");
        if (e.type == M_ERROR)
            n = tr("error");
        n += " " + tr("line") + " " + QString::number(e.line + 1) + ": " + e.text;

        auto item = new QListWidgetItem(messageList);
        item->setText(n);
        item->setData(Qt::UserRole + 1, e.line);
        messageList->addItem(item);
    }
}


void AbstractScriptWidget::addCompileMessage(int line, MessageType type, const QString &text)
{
    p_sw_->messages << PrivateSW::Message(line, type, text);
}


int AbstractScriptWidget::PrivateSW::posForLine(int line) const
{
    auto str = editor->toPlainText();

    int pos = 0, l = 0;
    while (l < line)
    {
        int idx = str.indexOf('\n', pos);
        if (idx < 0)
            return pos + 1;
        pos = idx + 1;
        ++l;
    }
    return pos;
}

} // namespace GUI
} // namespace MO
