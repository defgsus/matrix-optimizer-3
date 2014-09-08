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

#include "infowindow.h"
#include "io/application.h"

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
}


void InfoWindow::createWidgets_()
{
    setCentralWidget(new QWidget(this));
    auto lv = new QVBoxLayout(centralWidget());

        auto labelId = new QLabel(this);
        lv->addWidget(labelId);
        labelId->setFont(bigFont_);
        labelId->setText("1");
        labelId->setAlignment(Qt::AlignCenter);

        auto labelInfo = new QLabel(this);
        lv->addWidget(labelInfo);
        labelInfo->setWordWrap(true);
        labelInfo->setText(infoString());
}


QString InfoWindow::infoString() const
{
    QString text;
    QTextStream s(&text);

    s << application->applicationName()
      << "\nlocal ip: " << application->ipName();

    return text;
}


} // namespace GUI
} // namespace MO
