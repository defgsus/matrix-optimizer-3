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
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QMessageBox>

#include "geometrydialog.h"
#include "gl/geometry.h"
#include "gl/geometryfactory.h"
#include "widget/geometrywidget.h"
#include "widget/doublespinbox.h"
#include "widget/spinbox.h"
#include "tool/stringmanip.h"
#include "util/geometrycreator.h"


namespace MO {
namespace GUI {

GeometryDialog::GeometryDialog(QWidget *parent, Qt::WindowFlags flags) :
    QDialog         (parent, flags),
    settings_       (new GL::GeometryFactorySettings()),
    creator_        (0),
    updateGeometryLater_(false)
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

                // filename
                editFilename_ = new QLineEdit(this);
                lh2->addWidget(editFilename_);
                editFilename_->setText(settings_->filename);
                editFilename_->setReadOnly(true);

                butLoadModelFile_ = new QToolButton(this);
                lh2->addWidget(butLoadModelFile_);
                butLoadModelFile_->setText("...");
                connect(butLoadModelFile_, SIGNAL(clicked()), this, SLOT(loadModelFile_()));

            lh2 = new QHBoxLayout();
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

            lh2 = new QHBoxLayout();
            lv->addLayout(lh2);

                cbCalcNormals_ = new QCheckBox(tr("calculate normals"), this);
                lh2->addWidget(cbCalcNormals_);
                cbCalcNormals_->setChecked(settings_->calcNormals);
                connect(cbCalcNormals_, SIGNAL(stateChanged(int)),
                        this, SLOT(updateFromWidgets_()));

                cbNorm_ = new QCheckBox(tr("normalize coordinates"), this);
                lh2->addWidget(cbNorm_);
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

                spinSegX_ = new SpinBox(this);
                lh2->addWidget(spinSegX_);
                spinSegX_->setStatusTip("Number of segments (X)");
                spinSegX_->setRange(1, 10000);
                spinSegX_->setValue(settings_->segmentsX);
                connect(spinSegX_, SIGNAL(valueChanged(int)),
                        this, SLOT(updateFromWidgets_()));

                spinSegY_ = new SpinBox(this);
                lh2->addWidget(spinSegY_);
                spinSegY_->setStatusTip("Number of segments (Y)");
                spinSegY_->setRange(1, 10000);
                spinSegY_->setValue(settings_->segmentsY);
                connect(spinSegY_, SIGNAL(valueChanged(int)),
                        this, SLOT(updateFromWidgets_()));

                spinSegZ_ = new SpinBox(this);
                lh2->addWidget(spinSegZ_);
                spinSegZ_->setStatusTip("Number of segments (Z)");
                spinSegZ_->setRange(0, 10000);
                spinSegZ_->setValue(settings_->segmentsZ);
                connect(spinSegZ_, SIGNAL(valueChanged(int)),
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

                // remove randomly
                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    cbRemove_ = new QCheckBox(tr("randomly\nremove primitives"), this);
                    lh2->addWidget(cbRemove_);
                    cbTess_->setChecked(settings_->removeRandomly);
                    connect(cbRemove_, SIGNAL(toggled(bool)),
                            this, SLOT(updateFromWidgets_()));

                    spinRemoveProb_ = new DoubleSpinBox(this);
                    lh2->addWidget(spinRemoveProb_);
                    spinRemoveProb_->setStatusTip("Probability for removing points");
                    spinRemoveProb_->setDecimals(5);
                    spinRemoveProb_->setSingleStep(0.005);
                    spinRemoveProb_->setRange(0.0, 1.0);
                    spinRemoveProb_->setValue(settings_->removeProb);
                    connect(spinRemoveProb_, SIGNAL(valueChanged(double)),
                            this, SLOT(updateFromWidgets_()));

                    spinRemoveSeed_ = new SpinBox(this);
                    lh2->addWidget(spinRemoveSeed_);
                    spinRemoveSeed_->setStatusTip("Random seed for removing primitives");
                    spinRemoveSeed_->setRange(0, 10000000);
                    spinRemoveSeed_->setValue(settings_->removeSeed);
                    connect(spinRemoveSeed_, SIGNAL(valueChanged(int)),
                            this, SLOT(updateFromWidgets_()));

            lv->addStretch(1);

            labelInfo_ = new QLabel(this);
            lv->addWidget(labelInfo_);
}

void GeometryDialog::updateGeometry_()
{
    if (creator_)
    {
        updateGeometryLater_ = true;
        return;
    }

    updateGeometryLater_ = false;

    creator_ = new UTIL::GeometryCreator(this);
    creator_->setSettings(*settings_);

    connect(creator_, SIGNAL(failed(QString)),
            this, SLOT(creationFailed_(QString)));
    connect(creator_, SIGNAL(finished()), this, SLOT(creationFinished_()));

    creator_->start();
}

void GeometryDialog::creationFailed_(const QString & text)
{
    QMessageBox::critical(this, tr("Geometry creation"),
                          tr("Error creating geometry\n%1").arg(text));
    creator_->deleteLater();
    creator_ = 0;
    if (updateGeometryLater_)
        updateGeometry_();
}

void GeometryDialog::creationFinished_()
{
    auto g = creator_->takeGeometry();

    geoWidget_->setGeometry(g);

    labelInfo_->setText(tr("vertices: %1\ntriangles: %2\nlines: %3\nmemory: %4")
                        .arg(g->numVertices())
                        .arg(g->numTriangles())
                        .arg(g->numLines())
                        .arg(byte_to_string(g->memory())));

    creator_->deleteLater();
    creator_ = 0;
    if (updateGeometryLater_)
        updateGeometry_();
}

void GeometryDialog::updateFromWidgets_()
{
    // update settings

    if (comboType_->currentIndex() >= 0)
    settings_->type = (GL::GeometryFactorySettings::Type)
                        comboType_->itemData(comboType_->currentIndex()).toInt();

    settings_->filename = editFilename_->text();
    settings_->calcNormals = cbCalcNormals_->isChecked();
    settings_->asTriangles = cbTriangles_->isChecked();
    settings_->convertToLines = cbConvertToLines_->isChecked();
    settings_->sharedVertices = cbSharedVert_->isChecked();
    settings_->normalizeVertices = cbNorm_->isChecked();
    settings_->scale = spinS_->value();
    settings_->scaleX = spinSX_->value();
    settings_->scaleY = spinSY_->value();
    settings_->scaleZ = spinSZ_->value();
    settings_->segmentsX = spinSegX_->value();
    settings_->segmentsY = spinSegY_->value();
    settings_->segmentsZ = spinSegZ_->value();
    settings_->tesselate = cbTess_->isChecked();
    settings_->tessLevel = spinTess_->value();
    settings_->removeRandomly = cbRemove_->isChecked();
    settings_->removeProb = spinRemoveProb_->value();
    settings_->removeSeed = spinRemoveSeed_->value();

    // update widgets visibility

    const bool
            isFile = settings_->type ==
                    GL::GeometryFactorySettings::T_FILE,
            canTriangle = (settings_->type !=
                            GL::GeometryFactorySettings::T_GRID_XZ
                            && settings_->type !=
                            GL::GeometryFactorySettings::T_GRID),
            hasTriangle = (canTriangle && settings_->asTriangles),
            hasSegments = (settings_->type ==
                            GL::GeometryFactorySettings::T_UV_SPHERE
                           || settings_->type ==
                            GL::GeometryFactorySettings::T_GRID_XZ
                           || settings_->type ==
                            GL::GeometryFactorySettings::T_GRID),
            has3Segments = (hasSegments && settings_->type ==
                            GL::GeometryFactorySettings::T_GRID);

    cbTriangles_->setVisible( canTriangle && !isFile);

    cbCalcNormals_->setVisible( hasTriangle && !settings_->convertToLines );
    cbConvertToLines_->setVisible( hasTriangle );
    spinTess_->setVisible( settings_->tesselate );

    editFilename_->setVisible(isFile);
    butLoadModelFile_->setVisible(isFile);

    labelSeg_->setVisible( hasSegments );
    spinSegX_->setVisible( hasSegments );
    spinSegY_->setVisible( hasSegments );
    spinSegZ_->setVisible( has3Segments );

    spinRemoveProb_->setVisible( cbRemove_->isChecked() );
    spinRemoveSeed_->setVisible( cbRemove_->isChecked() );

    updateGeometry_();
}

void GeometryDialog::loadModelFile_()
{
    QString filename =
    QFileDialog::getOpenFileName(this, tr("Load .obj model"),
                                 "./", "Wavefront OBJ { *.obj }, All files { * }");
    if (filename.isEmpty())
        return;

    editFilename_->setText(filename);
    updateFromWidgets_();
}

} // namespace GUI
} // namespace MO
