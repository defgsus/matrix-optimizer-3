/** @file

    @brief widget for projector setup

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 2014/04/21</p>
*/

#include "projectorsetupwidget.h"
#include "ui_projectorsetupwidget.h"

ProjectorSetupWidget::ProjectorSetupWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProjectorSetupWidget)
{
    ui->setupUi(this);
}

ProjectorSetupWidget::~ProjectorSetupWidget()
{
    delete ui;
}
