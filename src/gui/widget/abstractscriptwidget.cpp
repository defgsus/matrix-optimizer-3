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
#include <QMessageBox>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>

#include "abstractscriptwidget.h"
#include "gui/helpdialog.h"
#include "io/files.h"
#include "io/settings.h"
#include "io/log.h"


namespace MO {
namespace GUI {

class AbstractScriptWidget::PrivateSW
{
public:

    PrivateSW(AbstractScriptWidget * widget)
        : widget            (widget)
        , isValid           (false)
        , ignoreTextChange  (false)
        , isUpdateOptional  (false)
        , isAlwaysUpdate    (false)
        , isChanged         (false)
    {
        QMutexLocker lock(&instanceMutex);
        instanceId = instanceCount++;
        instanceMap.insert(instanceId, widget);
    }

    ~PrivateSW()
    {
        QMutexLocker lock(&instanceMutex);
        instanceMap.remove(instanceId);
    }


    void createWidgets();
    void updateEditorColor();
    void updateErrorWidget();
    void updateInfoWidget();

    void onTextChanged(bool alwaysSend = false);

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

    bool isValid, ignoreTextChange,
        isUpdateOptional, isAlwaysUpdate,
        isChanged;

    IO::FileType fileType;
    QString curText, curFilename;

    QList<Message> messages;

    int instanceId;
    static QMutex instanceMutex;
    static QMap<int, AbstractScriptWidget*> instanceMap;
    static int instanceCount;
};

QMutex AbstractScriptWidget::PrivateSW::instanceMutex(QMutex::Recursive);
QMap<int, AbstractScriptWidget*> AbstractScriptWidget::PrivateSW::instanceMap;
int AbstractScriptWidget::PrivateSW::instanceCount = 0;

AbstractScriptWidget* AbstractScriptWidget::instanceForId(int id)
{
    QMutexLocker lock(&PrivateSW::instanceMutex);
    auto i = PrivateSW::instanceMap.find(id);
    if (i == PrivateSW::instanceMap.end())
        return nullptr;
    return i.value();
}

AbstractScriptWidget* AbstractScriptWidget::instanceForScriptText(const QString& txt)
{
    QMutexLocker lock(&PrivateSW::instanceMutex);
    for (auto w : PrivateSW::instanceMap)
    {
        if (txt == w->scriptText())
            return w;
    }
    return nullptr;
}

/** Returns a runtime-unique id for each widget */
int AbstractScriptWidget::instanceId() const { return p_sw_->instanceId; }

/** @todo TextEditDialog duplicates update functionality from AbstractScriptWidget and
            it's a bit more involving to work on that */
AbstractScriptWidget::AbstractScriptWidget(IO::FileType type, QWidget *parent)
    : QWidget       (parent),
      p_sw_         (new PrivateSW(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    p_sw_->fileType = type;
    p_sw_->createWidgets();
    p_sw_->updateInfoWidget();

    connect(this, SIGNAL(compileMessageAdded(int,int,QString)),
            this, SLOT(onAddCompileMessage(int,int,QString)),
            Qt::QueuedConnection);

    QFont font = p_sw_->editor->font();
    font.setPointSizeF( settings()->value(
                QString("ScriptEdit/%1/fontSize").arg(IO::fileTypeIds[p_sw_->fileType]),
                    font.pointSizeF()).toDouble() );
    setEditorFont(font, false);
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

void AbstractScriptWidget::setAlwaysUpdate(bool enable)
{
    p_sw_->isAlwaysUpdate = enable;
}

const QString AbstractScriptWidget::scriptText() const
{
    if (!p_sw_->timer->isActive())
        return p_sw_->curText;
    // return live content (when timer is still ticking)
    return p_sw_->editor->toPlainText();
}

void AbstractScriptWidget::setScriptText(const QString & t)
{
    p_sw_->timer->stop();
    p_sw_->isValid = false;
    p_sw_->curText = t;
    p_sw_->editor->setPlainText(t);
    p_sw_->isChanged = false;
}

void AbstractScriptWidget::updateScript()
{
    p_sw_->onTextChanged(true);
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
        /// @todo AbstractScriptWidget editor is squeezed by the message list
        editor->setMinimumHeight(400);
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


        // --- setup font ---

        // XXX does not work for Mac
        QFont f("Monospace");
        f.setStyleHint(QFont::Monospace);
        widget->setEditorFont(f, false);

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

        lInfo = new QLabel(widget);
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
            connect(butUpdate, &QPushButton::clicked, [this]() { onTextChanged(true); } );


        timer = new QTimer(widget);
        timer->setSingleShot(true);
        timer->setInterval(750);
        connect(timer, &QTimer::timeout, [=]() { onTextChanged(false); } );
}


void AbstractScriptWidget::PrivateSW::onTextChanged(bool alwaysSend)
{
    timer->stop();
    ignoreTextChange = false;

    QString tmp = curText;
    curText = editor->toPlainText();

    // only compile when changed
    if (tmp != curText)
    {
        isChanged = true;
        messages.clear();
        isValid = widget->compile();
    }

    updateEditorColor();
    updateErrorWidget();

    if (isValid)
        if (alwaysSend || isAlwaysUpdate
            || (isUpdateOptional && cbAlwaysUpdate->isChecked()))
            emit widget->scriptTextChanged();
}

void AbstractScriptWidget::keyPressEvent(QKeyEvent * e)
{
    const bool ctrl = (e->modifiers() & Qt::CTRL);

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

    if ((e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
        && (e->modifiers() & Qt::ALT || ctrl))
    {
        p_sw_->onTextChanged(true);
        e->accept();
        return;
    }

    if (ctrl && e->key() == Qt::Key_L)
    {
        loadScript();
        e->accept();
        return;
    }

    if (ctrl && e->key() == Qt::Key_S)
    {
        saveScript();
        e->accept();
        return;
    }

    if (ctrl && (e->modifiers() & Qt::SHIFT) && e->key() == Qt::Key_S)
    {
        saveScriptAs();
        e->accept();
        return;
    }

    if (ctrl && (e->key() == Qt::Key_Plus || e->text() == "+")) incFontSize();
    if (ctrl && (e->key() == Qt::Key_Minus || e->text() == "-")) decFontSize();

    QWidget::keyPressEvent(e);
}

void AbstractScriptWidget::incFontSize()
{
    auto font = p_sw_->editor->font();
    font.setPointSizeF(font.pointSizeF() * 1.1);
    setEditorFont(font, true);
}

void AbstractScriptWidget::decFontSize()
{
    auto font = p_sw_->editor->font();
    font.setPointSizeF(font.pointSizeF() / 1.1);
    setEditorFont(font, true);
}

void AbstractScriptWidget::setEditorFont(const QFont& f, bool doSave)
{
    p_sw_->editor->setFont(f);
    p_sw_->editor->setTabStopWidth(QFontMetrics(f).width("    "));
    if (doSave)
        settings()->setValue(
                QString("ScriptEdit/%1/fontSize").arg(IO::fileTypeIds[p_sw_->fileType]),
                    f.pointSizeF());
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
    QTextCursor c = editor->textCursor();
    QString text = tr("%1:%2 (char %3)")
                   .arg(c.blockNumber() + 1)
                   .arg(c.columnNumber() + 1)
                   .arg(c.position() + 1);
    QString sel = c.selectedText();
    if (sel.size())
        text += " " + tr("(%1 chars selected)").arg(sel.size());
    lInfo->setText(text);
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
        n += " " + tr("line") + " " + QString::number(e.line) + ": " + e.text;

        auto item = new QListWidgetItem(messageList);
        item->setText(n);
        item->setData(Qt::UserRole + 1, e.line);
        messageList->addItem(item);
    }
}


void AbstractScriptWidget::addCompileMessage(
        int line, MessageType type, const QString &text)
{
    MO_PRINT("COMPILE " << text);
    emit compileMessageAdded(line, type, text);
}

void AbstractScriptWidget::onAddCompileMessage(int line, int type, const QString &text)
{
    p_sw_->messages << PrivateSW::Message(line, MessageType(type), text);
    p_sw_->updateErrorWidget();
}


int AbstractScriptWidget::PrivateSW::posForLine(int line) const
{
    auto str = editor->toPlainText();

    int pos = 0, l = 1;
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

IO::FileType AbstractScriptWidget::fileType() const
{
    return p_sw_->fileType;
}

bool AbstractScriptWidget::isSaveToChange()
{
    if (!p_sw_->isChanged)
        return true;

    int a = QMessageBox::question(
                this, tr("Keep script?"),
                tr("The current script has not been stored, "
                   "do you want to loose the changes?"),
                tr("Discard"), tr("Keep"));
    if (a == 1)
        return false;
    return true;
}

bool AbstractScriptWidget::saveScript(const QString &fn)
{
    QFile f(fn);
    if (!f.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::critical(this, tr("Save script"),
                              tr("Error saving script %1\n%2")
                              .arg(fn).arg(f.errorString()));
        return false;
    }

    auto data = p_sw_->editor->toPlainText().toUtf8();
    if (f.write(data) != data.size())
    {
        QMessageBox::critical(this, tr("Save script"),
                              tr("Error saving script %1\n%2")
                              .arg(fn).arg(f.errorString()));
        return false;
    }

    p_sw_->curFilename = fn;
    p_sw_->isChanged = false;
    return true;
}

bool AbstractScriptWidget::loadScript(const QString &fn)
{
    QFile f(fn);
    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::critical(this, tr("Load script"),
                              tr("Error loading script %1\n%2")
                              .arg(fn).arg(f.errorString()));
        return false;
    }

    auto data = f.readAll();
    setScriptText(QString::fromUtf8(data));

    p_sw_->curFilename = fn;
    p_sw_->isChanged = false;
    return true;
}

bool AbstractScriptWidget::loadScript()
{
    QString fn = IO::Files::getOpenFileName(fileType(), this);
    if (fn.isEmpty())
        return false;
    return loadScript(fn);
}

bool AbstractScriptWidget::saveScript()
{
    QString fn = p_sw_->curFilename;
    if (fn.isEmpty() || !saveScript(fn))
        return saveScriptAs();
    return saveScript(fn);
}

bool AbstractScriptWidget::saveScriptAs()
{
    QString fn = IO::Files::getSaveFileName(fileType(), this);
    if (fn.isEmpty())
        return false;
    return saveScript(fn);
}


} // namespace GUI
} // namespace MO
