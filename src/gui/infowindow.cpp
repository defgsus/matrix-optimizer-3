/** @file infowindow.cpp

    @brief Window to display the client information

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 9/8/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QFont>
#include <QTextStream>
#include <QHostInfo>

#include "infowindow.h"
#include "io/application.h"
#include "io/systeminfo.h"
#include "io/settings.h"

namespace MO {
namespace GUI {


InfoWindow::InfoWindow(QWidget *parent) :
    QMainWindow(parent)
{
    normalFont_ = font();
    normalFont_.setPointSize(25);
    setFont(normalFont_);

    bigFont_ = normalFont_;
    bigFont_.setPointSize(120);

    createWidgets_();
    updateInfo();
}

void InfoWindow::createWidgets_()
{
    setCentralWidget(new QWidget(this));
    auto lv = new QVBoxLayout(centralWidget());

        labelId_ = new QLabel(this);
        lv->addWidget(labelId_);
        labelId_->setFont(bigFont_);
        labelId_->setAlignment(Qt::AlignCenter);

        labelInfo_ = new QLabel(this);
        lv->addWidget(labelInfo_);
        labelInfo_->setWordWrap(true);
}


void InfoWindow::updateInfo()
{
    labelId_->setText(QString::number(settings->clientIndex()));
    labelInfo_->setText(infoString());
}


QString InfoWindow::infoString() const
{
    SystemInfo info;
    info.get();

    QString text;
    QTextStream s(&text);

    s << info.toString();

    return text;
}




} // namespace GUI
} // namespace MO
