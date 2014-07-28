/** @file geometrydialog.cpp

    @brief Editor for Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#include <QLayout>
#include <QComboBox>
#include <QLabel>
#include <QCheckBox>

#include "geometrydialog.h"
#include "gl/geometry.h"
#include "gl/geometryfactory.h"
#include "widget/geometrywidget.h"
#include "widget/doublespinbox.h"
#include "widget/spinbox.h"
#include "tool/stringmanip.h"

namespace MO {
namespace GUI {

GeometryDialog::GeometryDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog         (parent, flags),
    settings_       (new GL::GeometryFactorySettings())
    //geomChanged_    (false)
{
    setObjectName("_GeometryWidget");
    setWindowTitle(tr("geomtry editor"));

    setMinimumSize(800,400);

    createWidgets_();

//    geomChanged_ = true;
}

GeometryDialog::~GeometryDialog()
{
    delete settings_;
}

void GeometryDialog::createWidgets_()
{
    auto lh = new QHBoxLayout(this);


        geoWidget_ = new GeometryWidget(this);
        lh->addWidget(geoWidget_);
        geoWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        connect(geoWidget_, SIGNAL(glInitialized()), this, SLOT(updateFromWidgets_()));

        auto lv = new QVBoxLayout();
        lh->addLayout(lv);

            // geometry type

            comboType_ = new QComboBox(this);
            lv->addWidget(comboType_);
            comboType_->setStatusTip("Selects the type of geometry");

            for (uint i=0; i<settings_->numTypes; ++i)
            {
                comboType_->addItem(settings_->typeNames[i], i);
                if (settings_->type == (GL::GeometryFactorySettings::Type)i)
                    comboType_->setCurrentIndex(i);
            }

            connect(comboType_, SIGNAL(currentIndexChanged(QString)),
                    this, SLOT(updateFromWidgets_()));

            auto lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                // create triangles
                cbTriangles_ = new QCheckBox(tr("create triangles"), this);
                lh2->addWidget(cbTriangles_);
                cbTriangles_->setChecked(settings_->asTriangles);
                connect(cbTriangles_, SIGNAL(stateChanged(int)),
                        this, SLOT(updateFromWidgets_()));

                // convert to lines
                cbConvertToLines_ = new QCheckBox(tr("convert to lines"), this);
                lh2->addWidget(cbConvertToLines_);
                cbConvertToLines_->setChecked(settings_->convertToLines);
                connect(cbConvertToLines_, SIGNAL(stateChanged(int)),
                        this, SLOT(updateFromWidgets_()));

            // shared vertices
            cbSharedVert_ = new QCheckBox(tr("shared vertices"), this);
            lv->addWidget(cbSharedVert_);
            cbSharedVert_->setChecked(settings_->sharedVertices);
            connect(cbSharedVert_, SIGNAL(stateChanged(int)),
                    this, SLOT(updateFromWidgets_()));

            cbNorm_ = new QCheckBox(tr("normalize coordinates"), this);
            lv->addWidget(cbNorm_);
            cbNorm_->setChecked(settings_->normalizeVertices);
            connect(cbNorm_, SIGNAL(stateChanged(int)),
                    this, SLOT(updateFromWidgets_()));

            // scale
            auto l = new QLabel(tr("scale"), this);
            lv->addWidget(l);

            spinS_ = new DoubleSpinBox(this);
            lv->addWidget(spinS_);
            spinS_->setStatusTip("Overall scale of the model");
            spinS_->setDecimals(5);
            spinS_->setSingleStep(0.1);
            spinS_->setRange(0.0001, 1000000);
            spinS_->setValue(settings_->scale);
            connect(spinS_, SIGNAL(valueChanged(double)),
                    this, SLOT(updateFromWidgets_()));

            lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                spinSX_ = new DoubleSpinBox(this);
                lh2->addWidget(spinSX_);
                spinSX_->setStatusTip("X-scale of the model");
                spinSX_->setDecimals(5);
                spinSX_->setSingleStep(0.1);
                spinSX_->setRange(0.0001, 1000000);
                spinSX_->setValue(settings_->scaleX);
                connect(spinSX_, SIGNAL(valueChanged(double)),
                        this, SLOT(updateFromWidgets_()));

                spinSY_ = new DoubleSpinBox(this);
                lh2->addWidget(spinSY_);
                spinSY_->setStatusTip("Y-scale of the model");
                spinSY_->setDecimals(5);
                spinSY_->setSingleStep(0.1);
                spinSY_->setRange(0.0001, 1000000);
                spinSY_->setValue(settings_->scaleY);
                connect(spinSY_, SIGNAL(valueChanged(double)),
                        this, SLOT(updateFromWidgets_()));

                spinSZ_ = new DoubleSpinBox(this);
                lh2->addWidget(spinSZ_);
                spinSZ_->setStatusTip("Z-scale of the model");
                spinSZ_->setDecimals(5);
                spinSZ_->setSingleStep(0.1);
                spinSZ_->setRange(0.0001, 1000000);
                spinSZ_->setValue(settings_->scaleZ);
                connect(spinSZ_, SIGNAL(valueChanged(double)),
                        this, SLOT(updateFromWidgets_()));


            // segments
            labelSeg_ = new QLabel(tr("segments"), this);
            lv->addWidget(labelSeg_);

            lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                spinSegU_ = new SpinBox(this);
                lh2->addWidget(spinSegU_);
                spinSegU_->setStatusTip("Number of segments (U)");
                spinSegU_->setRange(1, 10000);
                spinSegU_->setValue(settings_->segmentsU);
                connect(spinSegU_, SIGNAL(valueChanged(int)),
                        this, SLOT(updateFromWidgets_()));

                spinSegV_ = new SpinBox(this);
                lh2->addWidget(spinSegV_);
                spinSegV_->setStatusTip("Number of segments (V)");
                spinSegV_->setRange(1, 10000);
                spinSegV_->setValue(settings_->segmentsV);
                connect(spinSegV_, SIGNAL(valueChanged(int)),
                        this, SLOT(updateFromWidgets_()));

            // tesselation
            lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                cbTess_ = new QCheckBox(tr("tesselate"), this);
                lh2->addWidget(cbTess_);
                cbTess_->setChecked(settings_->tesselate);
                connect(cbTess_, SIGNAL(toggled(bool)),
                        this, SLOT(updateFromWidgets_()));

                spinTess_ = new SpinBox(this);
                lh2->addWidget(spinTess_);
                spinTess_->setStatusTip("Level of tesselation");
                spinTess_->setRange(1, 10);
                spinTess_->setValue(settings_->tessLevel);
                connect(spinTess_, SIGNAL(valueChanged(int)),
                        this, SLOT(updateFromWidgets_()));


            lv->addStretch(1);

            labelInfo_ = new QLabel(this);
            lv->addWidget(labelInfo_);
}
/*
void GeometryDialog::paintEvent(QPaintEvent * e)
{
    QDialog::paintEvent(e);

    if (geomChanged_)
    {
        geomChanged_ = false;
        //updateGeometry_();
    }
}
*/
void GeometryDialog::updateGeometry_()
{
    auto g = new GL::Geometry();
    GL::GeometryFactory::createFromSettings(g, settings_);
    geoWidget_->setGeometry(g);

    labelInfo_->setText(tr("vertices: %1\ntriangles: %2\nlines: %3\nmemory: %4")
                        .arg(g->numVertices())
                        .arg(g->numTriangles())
                        .arg(g->numLines())
                        .arg(byte_to_string(g->memory())));
}

void GeometryDialog::updateFromWidgets_()
{
    // update settings

    if (comboType_->currentIndex() >= 0)
    settings_->type = (GL::GeometryFactorySettings::Type)
                        comboType_->itemData(comboType_->currentIndex()).toInt();

    settings_->asTriangles = cbTriangles_->isChecked();
    settings_->convertToLines = cbConvertToLines_->isChecked();
    settings_->sharedVertices = cbSharedVert_->isChecked();
    settings_->normalizeVertices = cbNorm_->isChecked();
    settings_->scale = spinS_->value();
    settings_->scaleX = spinSX_->value();
    settings_->scaleY = spinSY_->value();
    settings_->scaleZ = spinSZ_->value();
    settings_->segmentsU = spinSegU_->value();
    settings_->segmentsV = spinSegV_->value();
    settings_->tesselate = cbTess_->isChecked();
    settings_->tessLevel = spinTess_->value();

    // update widgets visibility

    const bool
            canTriangle = (settings_->type !=
                            GL::GeometryFactorySettings::T_GRID),
            hasTriangle = (canTriangle && settings_->asTriangles),
            hasSegments = (settings_->type ==
                            GL::GeometryFactorySettings::T_UV_SPHERE
                           || settings_->type ==
                            GL::GeometryFactorySettings::T_GRID);

    cbTriangles_->setVisible( canTriangle );

    cbConvertToLines_->setVisible( hasTriangle );
    cbTess_->setVisible( hasTriangle );
    spinTess_->setVisible( hasTriangle && cbTess_->isChecked() );

    labelSeg_->setVisible( hasSegments );
    spinSegU_->setVisible( hasSegments );
    spinSegV_->setVisible( hasSegments );

    updateGeometry_();
}

} // namespace GUI
} // namespace MO
