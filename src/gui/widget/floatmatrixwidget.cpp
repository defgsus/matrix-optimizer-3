/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/18/2016</p>
*/

#include <QTableWidget>
#include <QLayout>

#include "floatmatrixwidget.h"

namespace MO {
namespace GUI {

struct FloatMatrixWidget::Private
{
    Private(FloatMatrixWidget* p)
        : p         (p)
    { }

    FloatMatrixWidget* p;

    FloatMatrix matrix;
    size_t w, h;
    QTableWidget* table;

};

FloatMatrixWidget::FloatMatrixWidget(QWidget *parent)
    : QWidget   (parent)
    , p_        (new Private(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto lv = new QVBoxLayout(this);
    lv->setMargin(0);

        p_->table = new QTableWidget(this);
        lv->addWidget(p_->table);
}

FloatMatrixWidget::~FloatMatrixWidget()
{
    delete p_;
}

const FloatMatrix& FloatMatrixWidget::floatMatrix() const
{
    return p_->matrix;
}

void FloatMatrixWidget::setFloatMatrix(const FloatMatrix& m)
{
    p_->matrix = m;
    if (m.numDimensions() < 1)
    {
        p_->table->clear();
        p_->w = p_->h = 0;
    }
    else
    {
        p_->w = m.size(0);
        p_->h = m.numDimensions() >= 2 ? m.size(1) : 1;
        p_->table->setColumnCount(p_->w);
        p_->table->setRowCount(p_->h);
        for (size_t y = 0; y < p_->h; ++y)
        for (size_t x = 0; x < p_->w; ++x)
        {
            auto item = new QTableWidgetItem(tr("%1").arg(*p_->matrix.data(x, y)));
            item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled
                           | Qt::ItemIsSelectable);
            p_->table->setItem(y, x, item);
        }
    }
}





} // namespace GUI
} // namespace MO
