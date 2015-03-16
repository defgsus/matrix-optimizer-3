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
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>


#include "abstractscriptwidget.h"
#include "gui/helpdialog.h"
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
          ignoreTextChange(false),
          isUpdateOptional(false)
    { }

    void createWidgets();
    void updateEditorColor();
    void updateErrorWidget();
    void updateInfoWidget();

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
    QCheckBox * cbAlwaysUpdate;
    QPushButton * butUpdate;
    QLabel * lInfo;
    QTimer * timer;

    bool isValid, ignoreTextChange, isUpdateOptional;

    QString curText;

    QList<Message> messages;
};


AbstractScriptWidget::AbstractScriptWidget(QWidget *parent)
    : QWidget       (parent),
      p_sw_         (new PrivateSW(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    p_sw_->createWidgets();
    p_sw_->updateInfoWidget();
}

AbstractScriptWidget::~AbstractScriptWidget()
{
    delete p_sw_;
}

bool AbstractScriptWidget::isScriptValid() const
{
    return p_sw_->isValid;
}

void AbstractScriptWidget::setUpdateOptional(bool enable)
{
    p_sw_->isUpdateOptional = enable;
    p_sw_->butUpdate->setVisible(enable);
    p_sw_->cbAlwaysUpdate->setVisible(enable);
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
    lv->setMargin(0);

        // --- editor ---

        editor = new QPlainTextEdit(widget);
        editor->setProperty("code", true);
        editor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        // XXX
        editor->setMinimumHeight(500);
        lv->addWidget(editor, 10);
        connect(editor, &QPlainTextEdit::textChanged, [this]()
        {
            if (!ignoreTextChange)
                timer->start();
        });
        connect(editor, &QPlainTextEdit::cursorPositionChanged, [this]()
        {
            updateInfoWidget();
        });
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
#if QT_VERSION >= 0x050300
        messageList->setSizeAdjustPolicy(QListWidget::AdjustToContents);
#endif
        lv->addWidget(messageList);
        connect(messageList, &QListWidget::itemClicked, [=](QListWidgetItem*item)
        {
            int line = item->data(Qt::UserRole + 1).toInt();
            QTextCursor curs(editor->textCursor());
            curs.setPosition(posForLine(line));
            curs.select(QTextCursor::LineUnderCursor);
            editor->setTextCursor(curs);
        });

        lInfo = new QLabel("balbla", widget);
        lv->addWidget(lInfo);

        // -- optional update --

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            cbAlwaysUpdate = new QCheckBox(tr("always update"), widget);
            cbAlwaysUpdate->setVisible(isUpdateOptional);
            lh->addWidget(cbAlwaysUpdate);


            butUpdate = new QPushButton(tr("up&date"), widget);
            butUpdate->setVisible(isUpdateOptional);
            lh->addWidget(butUpdate);
            connect(butUpdate, SIGNAL(pressed()), widget, SIGNAL(scriptTextChanged()));


        timer = new QTimer(widget);
        timer->setSingleShot(true);
        timer->setInterval(600);
        connect(timer, &QTimer::timeout, [=]() { onTextChanged(); } );
}


void AbstractScriptWidget::PrivateSW::onTextChanged()
{
    QString tmp = curText;
    curText = editor->toPlainText();

    // only compile when new
    if (tmp != curText)
    {
        messages.clear();
        isValid = widget->compile();
    }

    updateEditorColor();
    updateErrorWidget();

    if (isValid)
        if (!isUpdateOptional || cbAlwaysUpdate->isChecked())
            emit widget->scriptTextChanged();
}

void AbstractScriptWidget::keyPressEvent(QKeyEvent * e)
{
    if (e->key() == Qt::Key_F1)
    {
        // get word under cursor
        auto c = p_sw_->editor->textCursor();
        c.select(QTextCursor::WordUnderCursor);
        // get help url
        QString url = getHelpUrl( c.selectedText() );

        HelpDialog::run(url);

        e->accept();
        return;
    }

    QWidget::keyPressEvent(e);
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

void AbstractScriptWidget::PrivateSW::updateInfoWidget()
{
    auto c = editor->textCursor();
    lInfo->setText(tr("pos %1:%2, char %3")
                   .arg(c.blockNumber())
                   .arg(c.columnNumber())
                   .arg(c.position())
                   );
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
    p_sw_->updateErrorWidget();
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
