/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/18/2016</p>
*/

#include <QTableWidget>
#include <QToolButton>
#include <QLayout>
#include <QMessageBox>
#include <QLabel>

#include "FloatMatrixWidget.h"
#include "gui/widget/SpinBox.h"
#include "math/FloatMatrix.h"
#include "types/float.h"
#include "io/Files.h"
#include "io/error.h"
#include "io/log.h"

namespace MO {
namespace GUI {

struct FloatMatrixWidget::Private
{
    Private(FloatMatrixWidget* p)
        : p             (p)
        , w             (0)
        , h             (0)
        , d             (0)
        , ignoreChange  (false)
        , isReadOnly    (false)
    { }

    void resize();
    void updateInfoLabel();
    void updateTableFromMatrix();
    void updateMatrixFromTable();
    bool updateMatrixFromCell(int r, int c);
    QTableWidgetItem* createItem(Double value) const;

    FloatMatrixWidget* p;

    FloatMatrix matrix;
    size_t w, h, d;
    bool ignoreChange, isReadOnly;
    QTableWidget* table;
    SpinBox *sbW, *sbH, *sbD,
            *sbZ;
    QToolButton *butResize, *butSave;
    QLabel* labelInfo;
};

FloatMatrixWidget::FloatMatrixWidget(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto lv = new QVBoxLayout(this);
    lv->setMargin(0);

        auto lh = new QHBoxLayout();
        lv->addLayout(lh);

            p_->sbW = new SpinBox(this);
            p_->sbW->setLabel(tr("width"));
            p_->sbW->setRange(1, 1<<30);
            lh->addWidget(p_->sbW);

            p_->sbH = new SpinBox(this);
            p_->sbH->setLabel(tr("height"));
            p_->sbH->setRange(1, 1<<30);
            lh->addWidget(p_->sbH);

            p_->sbD = new SpinBox(this);
            p_->sbD->setLabel(tr("depth"));
            p_->sbD->setRange(1, 1<<30);
            lh->addWidget(p_->sbD);

            p_->butResize = new QToolButton(this);
            p_->butResize->setText(tr("Resize"));
            lh->addWidget(p_->butResize);
            connect(p_->butResize, &QToolButton::clicked, [=]()
            {
                p_->resize();
            });

            lh->addStretch(1);

            p_->labelInfo = new QLabel(this);
            lh->addWidget(p_->labelInfo);

        lh = new QHBoxLayout();
        lv->addLayout(lh);

            p_->sbZ = new SpinBox(this);
            p_->sbZ->setLabel(tr("Z-pos"));
            p_->sbZ->setRange(0, 1<<30);
            lh->addWidget(p_->sbZ);
            connect(p_->sbZ, &SpinBox::valueChanged, [=]()
            {
                if (!p_->ignoreChange)
                    p_->updateTableFromMatrix();
            });

            lh->addStretch(1);

            auto but = p_->butSave = new QToolButton(this);
            but->setText(tr("Load"));
            but->setStatusTip(tr("Load a matrix from json data"));
            but->setShortcut(Qt::CTRL + Qt::Key_L);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked(bool)), this, SLOT(loadDialog()));

            but = new QToolButton(this);
            but->setText(tr("Save"));
            but->setStatusTip(tr("Saves the matrix to json data"));
            but->setShortcut(Qt::CTRL + Qt::Key_S);
            lh->addWidget(but);
            connect(but, SIGNAL(clicked(bool)), this, SLOT(saveDialog()));

        p_->table = new QTableWidget(this);
        lv->addWidget(p_->table);
        connect(p_->table, &QTableWidget::cellChanged, [=](int r, int c)
        {
            if (p_->ignoreChange)
                return;
            if (p_->updateMatrixFromCell(r, c))
            {
                p_->updateInfoLabel();
                emit matrixChanged();
            }

        });
}

FloatMatrixWidget::~FloatMatrixWidget()
{
    delete p_;
}

const FloatMatrix& FloatMatrixWidget::floatMatrix() const
{
    return p_->matrix;
}

void FloatMatrixWidget::setReadOnly(bool e)
{
    p_->isReadOnly = e;
    p_->butSave->setVisible(!p_->isReadOnly);
    p_->sbH->setEnabled(!p_->isReadOnly);
    p_->sbW->setEnabled(!p_->isReadOnly);
    p_->sbD->setEnabled(!p_->isReadOnly);
    p_->butResize->setEnabled(!p_->isReadOnly);
}

void FloatMatrixWidget::Private::updateInfoLabel()
{
    labelInfo->setText(QString::fromStdString(
        matrix.layoutString() + " [" + matrix.rangeString() + "]"));
}

void FloatMatrixWidget::setFloatMatrix(const FloatMatrix& m)
{
    p_->ignoreChange = true;

    p_->matrix = m;
    if (m.isEmpty())
    {
        p_->table->clear();
        p_->w = p_->h = 0;
        p_->sbW->setValue(0);
        p_->sbH->setValue(0);
        p_->sbD->setValue(0);
    }
    else
    {
        p_->updateTableFromMatrix();
        p_->sbW->setValue(p_->w);
        p_->sbH->setValue(p_->h);
        p_->sbD->setValue(p_->d);
    }
    p_->ignoreChange = false;
}

QTableWidgetItem* FloatMatrixWidget::Private::createItem(Double value) const
{
    auto item = new QTableWidgetItem(tr("%1").arg(value));
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled
                   | Qt::ItemIsSelectable);
    return item;
}

void FloatMatrixWidget::Private::resize()
{
    /*
    if ((int)w == sbW->value() && (int)h == sbH->value()
      && (int)d == sbD->value())
        return;
    */

    w = sbW->value();
    h = sbH->value();
    d = sbD->value();
    table->setColumnCount(w);
    table->setRowCount(h);
    for (size_t y = 0; y < h; ++y)
    for (size_t x = 0; x < w; ++x)
    {
        if (!table->item(y, x))
            table->setItem(y, x, createItem(0.0));
    }
    updateMatrixFromTable();

    emit p->matrixChanged();
}

void FloatMatrixWidget::Private::updateTableFromMatrix()
{
    const bool prevIgnore = ignoreChange;
    ignoreChange = true;
    updateInfoLabel();
    if (matrix.isEmpty())
    {
        table->clear();
    }
    else
    {
        switch (matrix.numDimensions())
        {
            case 1: w = matrix.size(0); h = d = 1; break;
            case 2: w = matrix.size(1); h = matrix.size(0); d = 1; break;
            case 3: w = matrix.size(2); h = matrix.size(1);
                    d = matrix.size(0); break;
            default: w = h = d = 1; break;
        }
        size_t z = std::max(0,std::min(int(d)-1, sbZ->value() ));
        table->setColumnCount(w);
        table->setRowCount(h);
        for (size_t y = 0; y < h; ++y)
        for (size_t x = 0; x < w; ++x)
        {
            if (matrix.numDimensions() == 1)
                table->setItem(y, x, createItem(matrix(x)));
            else if (matrix.numDimensions() == 2)
                table->setItem(y, x, createItem(matrix(y, x)));
            else if (matrix.numDimensions() == 3)
                table->setItem(y, x, createItem(matrix(z, y, x)));
        }
    }
    ignoreChange = prevIgnore;
}

void FloatMatrixWidget::Private::updateMatrixFromTable()
{
    if (table->rowCount() == 0 || table->columnCount() == 0)
    {
        matrix.clear();
        updateInfoLabel();
        return;
    }
    if (sbD->value() <= 1)
    {
        if (table->rowCount() == 1)
        {
            matrix.setDimensions({ (size_t)table->columnCount() });
            for (int i=0; i<table->columnCount(); ++i)
                matrix(i) = table->item(0, i)->text().toDouble();
        }
        else
        {
            matrix.setDimensions({ (size_t)table->rowCount(),
                                   (size_t)table->columnCount() });
            for (int j=0; j<table->rowCount(); ++j)
                for (int i=0; i<table->columnCount(); ++i)
                    *matrix.data(j, i) = table->item(j, i)->text().toDouble();
        }
    }
    else
    {
        matrix.setDimensions({ (size_t)sbD->value(),
                               (size_t)table->rowCount(),
                               (size_t)table->columnCount() });
        size_t z = std::max(0,std::min(sbD->value()-1, sbZ->value() ));
        for (int j=0; j<table->rowCount(); ++j)
            for (int i=0; i<table->columnCount(); ++i)
                matrix(z, j, i) = table->item(j, i)->text().toDouble();
    }
    updateInfoLabel();
}

bool FloatMatrixWidget::Private::updateMatrixFromCell(int r, int c)
{
    if (matrix.isEmpty())
        return false;
    if (c < 0 || (size_t)c >= matrix.size(matrix.numDimensions()-1))
        return false;
    if (matrix.numDimensions() == 1)
    {
        if (r != 0)
            return false;
        matrix(c) = table->item(r, c)->text().toDouble();
        return true;
    }
    else if (matrix.numDimensions() == 2)
    {
        if (r < 0 || (size_t)r >= matrix.size(0))
            return false;

        matrix(r, c) = table->item(r, c)->text().toDouble();
        return true;
    }
    else if (matrix.numDimensions() == 3)
    {
        if (r < 0 || (size_t)r >= matrix.size(1))
            return false;

        size_t z = std::max(0,std::min(int(matrix.size(2))-1,
                                       sbZ->value() ));
        matrix(z, r, c) = table->item(r, c)->text().toDouble();
        return true;
    }

    return false;
}

void FloatMatrixWidget::loadDialog()
{
    auto fn = IO::Files::getOpenFileName(IO::FT_FLOAT_MATRIX);
    if (fn.isEmpty())
        return;
    try
    {
        FloatMatrix m;
        m.loadJsonFile(fn);
        setFloatMatrix(m);
        emit matrixChanged();
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("load float matrix"),
                              tr("Loading the float matrix failed,\n%1")
                              .arg(e.what()));
    }
}

void FloatMatrixWidget::saveDialog()
{
    auto fn = IO::Files::getSaveFileName(IO::FT_FLOAT_MATRIX);
    if (fn.isEmpty())
        return;
    try
    {
        p_->matrix.saveJsonFile(fn);
    }
    catch (const Exception& e)
    {
        QMessageBox::critical(this, tr("save float matrix"),
                              tr("Saving the float matrix failed,\n%1")
                              .arg(e.what()));
    }
}



} // namespace GUI
} // namespace MO
