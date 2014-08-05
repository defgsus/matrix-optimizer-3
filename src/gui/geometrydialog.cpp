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
#include <QPushButton>
#include <QStatusBar>
#include <QStatusTipEvent>
#include <QProgressBar>

#include "geometrydialog.h"
#include "geom/geometry.h"
#include "geom/geometryfactory.h"
#include "widget/geometrywidget.h"
#include "widget/doublespinbox.h"
#include "widget/spinbox.h"
#include "tool/stringmanip.h"
#include "geom/geometrycreator.h"
#include "widget/equationeditor.h"
#include "io/files.h"

namespace MO {
namespace GUI {

GeometryDialog::GeometryDialog(const GEOM::GeometryFactorySettings *set,
                               QWidget *parent, Qt::WindowFlags flags) :
    QDialog         (parent, flags),
    settings_       (new GEOM::GeometryFactorySettings()),
    creator_        (0),
    updateGeometryLater_(false),
    ignoreUpdate_   (false)
{
    setObjectName("_GeometryWidget");
    setWindowTitle(tr("geometry editor"));

    setMinimumSize(960,400);

    if (set)
        *settings_ = *set;

    createWidgets_();

    updatePresetList_();

//    geomChanged_ = true;
}

GeometryDialog::~GeometryDialog()
{
    delete settings_;
}

bool GeometryDialog::event(QEvent * e)
{
    if (QStatusTipEvent * tip = dynamic_cast<QStatusTipEvent*>(e))
    {
        statusBar_->showMessage(tip->tip());
    }
    return QDialog::event(e);
}

void GeometryDialog::createWidgets_()
{
    auto lv0 = new QVBoxLayout(this);
    lv0->setMargin(0);

        auto lh = new QHBoxLayout();
        lv0->addLayout(lh);

            auto lv = new QVBoxLayout();
            lh->addLayout(lv);

                // geometry widget
                geoWidget_ = new GeometryWidget(GeometryWidget::RM_DIRECT, this);
                lv->addWidget(geoWidget_);
                geoWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
                connect(geoWidget_, SIGNAL(glInitialized()), this, SLOT(updateFromWidgets_()));

                // view settings
                auto lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    comboView_ = new QComboBox(this);
                    lh2->addWidget(comboView_);
                    comboView_->setStatusTip(tr("Selects the projection type of the geometry window"));
                    comboView_->addItem(tr("orthographic"), GeometryWidget::RM_DIRECT_ORTHO);
                    comboView_->addItem(tr("perspective"), GeometryWidget::RM_DIRECT);
                    comboView_->addItem(tr("fulldome cubemap"), GeometryWidget::RM_FULLDOME_CUBE);
                    comboView_->setCurrentIndex(1);
                    connect(comboView_, SIGNAL(currentIndexChanged(int)),
                            this, SLOT(changeView_()));

                    auto cb = new QCheckBox(tr("show coordinates"), this);
                    lh2->addWidget(cb);
                    cb->setChecked(geoWidget_->isShowGrid());
                    connect(cb, &QCheckBox::stateChanged, [this](int state)
                    {
                        geoWidget_->setShowGrid(state == Qt::Checked);
                    });

                    cb = new QCheckBox(tr("textured"), this);
                    lh2->addWidget(cb);
                    cb->setChecked(geoWidget_->isShowTexture());
                    connect(cb, &QCheckBox::stateChanged, [this](int state)
                    {
                        geoWidget_->setShowTexture(state == Qt::Checked);
                    });

            lv = new QVBoxLayout();
            lh->addLayout(lv);

                // preset dialog

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    comboPreset_ = new QComboBox(this);
                    lh2->addWidget(comboPreset_);
                    connect(comboPreset_, SIGNAL(currentIndexChanged(int)),
                            this, SLOT(presetSelected_()));

                    butSavePreset_ = new QToolButton(this);
                    lh2->addWidget(butSavePreset_);
                    butSavePreset_->setText("S");
                    connect(butSavePreset_, SIGNAL(clicked()),
                            this, SLOT(savePreset_()));

                    butSavePresetAs_ = new QToolButton(this);
                    lh2->addWidget(butSavePresetAs_);
                    butSavePresetAs_->setText("...");
                    connect(butSavePresetAs_, SIGNAL(clicked()),
                            this, SLOT(savePresetAs_()));

                    butDeletePreset_ = new QToolButton(this);
                    lh2->addWidget(butDeletePreset_);
                    butDeletePreset_->setIcon(QIcon(":/icon/delete.png"));
                    connect(butDeletePreset_, SIGNAL(clicked()),
                            this, SLOT(deletePreset_()));

                // geometry type

                comboType_ = new QComboBox(this);
                lv->addWidget(comboType_);
                comboType_->setStatusTip("Selects the type of geometry");

                for (uint i=0; i<settings_->numTypes; ++i)
                {
                    comboType_->addItem(settings_->typeNames[i], i);
                    if (settings_->type == (GEOM::GeometryFactorySettings::Type)i)
                        comboType_->setCurrentIndex(i);
                }

                connect(comboType_, SIGNAL(currentIndexChanged(QString)),
                        this, SLOT(updateFromWidgets_()));

                lh2 = new QHBoxLayout();
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

                // normals and normalization
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

                // normalization amount
                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    labelNormAmt_ = new QLabel(tr("normalization"), this);
                    lh2->addWidget(labelNormAmt_);

                    spinNormAmt_ = new DoubleSpinBox(this);
                    lh2->addWidget(spinNormAmt_);
                    spinNormAmt_->setStatusTip("Amount of normalization between 0 and 1");
                    spinNormAmt_->setDecimals(5);
                    spinNormAmt_->setSingleStep(0.02);
                    spinNormAmt_->setRange(0.0, 1);
                    spinNormAmt_->setValue(settings_->normalization);
                    connect(spinNormAmt_, SIGNAL(valueChanged(double)),
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

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    labelSmallRadius_ = new QLabel(tr("small radius"), this);
                    lh2->addWidget(labelSmallRadius_);

                    spinSmallRadius_ = new DoubleSpinBox(this);
                    lh2->addWidget(spinSmallRadius_);
                    spinSmallRadius_->setStatusTip("Smaller radius");
                    spinSmallRadius_->setDecimals(5);
                    spinSmallRadius_->setSingleStep(0.02);
                    spinSmallRadius_->setRange(0.0001, 100000);
                    spinSmallRadius_->setValue(settings_->smallRadius);
                    connect(spinSmallRadius_, SIGNAL(valueChanged(double)),
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
                    cbRemove_->setChecked(settings_->removeRandomly);
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

                // transform by equation

                cbTransformEqu_ = new QCheckBox(tr("transform by equation"), this);
                lv->addWidget(cbTransformEqu_);
                cbTransformEqu_->setStatusTip(tr("Enables transformation of each vertex point "
                                                 "by a mathematical formula"));
                cbTransformEqu_->setChecked(settings_->transformWithEquation);
                connect(cbTransformEqu_, SIGNAL(toggled(bool)),
                        this, SLOT(updateFromWidgets_()));

                QStringList vars = { "x", "y", "z", "i" };
                editEquX_ = new EquationEditor(this);
                lv->addWidget(editEquX_);
                editEquX_->addVariables(vars);
                editEquX_->setPlainText(settings_->equationX);
                connect(editEquX_, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

                editEquY_ = new EquationEditor(this);
                lv->addWidget(editEquY_);
                editEquY_->addVariables(vars);
                editEquY_->setPlainText(settings_->equationY);
                connect(editEquY_, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

                editEquZ_ = new EquationEditor(this);
                lv->addWidget(editEquZ_);
                editEquZ_->addVariables(vars);
                editEquZ_->setPlainText(settings_->equationZ);
                connect(editEquZ_, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

                // transform primitives by equation

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    cbTransformPrimEqu_ = new QCheckBox(tr("transform primitives by equation"), this);
                    lh2->addWidget(cbTransformPrimEqu_);
                    cbTransformPrimEqu_->setStatusTip(tr("Enables transformation of each vertex point "
                                                     "of each primitive by a mathematical formula"));
                    cbTransformPrimEqu_->setChecked(settings_->transformPrimitivesWithEquation);
                    connect(cbTransformPrimEqu_, SIGNAL(toggled(bool)),
                            this, SLOT(updateFromWidgets_()));

                    cbCalcNormalsBeforePrimEqu_ = new QCheckBox(tr("calculate normals"), this);
                    lh2->addWidget(cbCalcNormalsBeforePrimEqu_);
                    cbCalcNormalsBeforePrimEqu_->setStatusTip(tr("Enables calculation of normals before "
                                                         "the application of the primitive equations"));
                    cbCalcNormalsBeforePrimEqu_->setChecked(settings_->transformPrimitivesWithEquation);
                    connect(cbCalcNormalsBeforePrimEqu_, SIGNAL(toggled(bool)),
                            this, SLOT(updateFromWidgets_()));

                vars = {
                    "x", "y", "z", "nx", "ny", "nz", "i", "p",
                    "x1", "y1", "z1", "x2", "y2", "z2", "x3", "y3", "z3",
                    "nx1", "ny1", "nz1", "nx2", "ny2", "nz2", "nx3", "ny3", "nz3" };
                editPEquX_ = new EquationEditor(this);
                lv->addWidget(editPEquX_);
                editPEquX_->addVariables(vars);
                editPEquX_->setPlainText(settings_->pEquationX);
                connect(editPEquX_, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

                editPEquY_ = new EquationEditor(this);
                lv->addWidget(editPEquY_);
                editPEquY_->addVariables(vars);
                editPEquY_->setPlainText(settings_->pEquationY);
                connect(editPEquY_, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

                editPEquZ_ = new EquationEditor(this);
                lv->addWidget(editPEquZ_);
                editPEquZ_->addVariables(vars);
                editPEquZ_->setPlainText(settings_->pEquationZ);
                connect(editPEquZ_, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

                // -----------------
                lv->addStretch(1);

                // info label
                labelInfo_ = new QLabel(this);
                lv->addWidget(labelInfo_);

                // OK/Cancel

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    auto but = new QPushButton(tr("Ok"), this);
                    lh2->addWidget(but);
                    connect(but, SIGNAL(clicked()), this, SLOT(accept()));

                    but = new QPushButton(tr("Cancel"), this);
                    lh2->addWidget(but);
                    connect(but, SIGNAL(clicked()), this, SLOT(reject()));

        statusBar_ = new QStatusBar(this);
        lv0->addWidget(statusBar_);

        progressBar_ = new QProgressBar(this);
        progressBar_->setOrientation(Qt::Horizontal);
        progressBar_->setRange(0,100);
        progressBar_->setVisible(false);
        statusBar_->addPermanentWidget(progressBar_);
}

void GeometryDialog::changeView_()
{
    if (comboView_->currentIndex() < 0)
        return;

    GeometryWidget::RenderMode rm = (GeometryWidget::RenderMode)
            comboView_->itemData(comboView_->currentIndex()).toInt();

    geoWidget_->setRenderMode(rm);
    /*
    QLayout * l = geoWidget_->layout();
    geoWidget_->setVisible(false);
    geoWidget_->deleteLater();

    // recreate
    geoWidget_ = new GeometryWidget(rm, this);
    l->addWidget(geoWidget_);
    geoWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(geoWidget_, SIGNAL(glInitialized()), this, SLOT(updateFromWidgets_()));
    */
}

void GeometryDialog::updateGeometry_()
{
    if (creator_)
    {
        updateGeometryLater_ = true;
        return;
    }

    updateGeometryLater_ = false;

    creator_ = new GEOM::GeometryCreator(this);
    creator_->setSettings(*settings_);

    connect(creator_, SIGNAL(failed(QString)),
            this, SLOT(creationFailed_(QString)));
    connect(creator_, SIGNAL(finished()), this, SLOT(creationFinished_()));
    connect(creator_, SIGNAL(progress(int)),
            this, SLOT(creatorProgress_(int)));

    progressBar_->setVisible(true);
    creator_->start();
}

void GeometryDialog::creatorProgress_(int p)
{
    progressBar_->setValue(p);
}

void GeometryDialog::creationFailed_(const QString & text)
{
    QMessageBox::critical(this, tr("Geometry creation"),
                          tr("Error creating geometry\n%1").arg(text));
    creator_->deleteLater();
    creator_ = 0;
    progressBar_->setVisible(false);
    if (updateGeometryLater_)
        updateGeometry_();
}

void GeometryDialog::creationFinished_()
{
    auto g = creator_->takeGeometry();

    geoWidget_->setGeometry(g);

    progressBar_->setVisible(false);
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
    if (ignoreUpdate_)
        return;

    // update settings

    if (comboType_->currentIndex() >= 0)
    settings_->type = (GEOM::GeometryFactorySettings::Type)
                        comboType_->itemData(comboType_->currentIndex()).toInt();

    settings_->filename = editFilename_->text();
    settings_->calcNormals = cbCalcNormals_->isChecked();
    settings_->asTriangles = cbTriangles_->isChecked();
    settings_->convertToLines = cbConvertToLines_->isChecked();
    settings_->sharedVertices = cbSharedVert_->isChecked();
    settings_->normalizeVertices = cbNorm_->isChecked();
    settings_->normalization = spinNormAmt_->value();
    settings_->scale = spinS_->value();
    settings_->scaleX = spinSX_->value();
    settings_->scaleY = spinSY_->value();
    settings_->scaleZ = spinSZ_->value();
    settings_->smallRadius = spinSmallRadius_->value();
    settings_->segmentsX = spinSegX_->value();
    settings_->segmentsY = spinSegY_->value();
    settings_->segmentsZ = spinSegZ_->value();
    settings_->tesselate = cbTess_->isChecked();
    settings_->tessLevel = spinTess_->value();
    settings_->removeRandomly = cbRemove_->isChecked();
    settings_->removeProb = spinRemoveProb_->value();
    settings_->removeSeed = spinRemoveSeed_->value();
    settings_->transformWithEquation = cbTransformEqu_->isChecked();
    settings_->transformPrimitivesWithEquation = cbTransformPrimEqu_->isChecked();
    settings_->calcNormalsBeforePrimitiveEquation = cbCalcNormalsBeforePrimEqu_->isChecked();
    if (editEquX_->isOk())
        settings_->equationX = editEquX_->toPlainText();
    if (editEquY_->isOk())
        settings_->equationY = editEquY_->toPlainText();
    if (editEquZ_->isOk())
        settings_->equationZ = editEquZ_->toPlainText();
    if (editPEquX_->isOk())
        settings_->pEquationX = editPEquX_->toPlainText();
    if (editPEquY_->isOk())
        settings_->pEquationY = editPEquY_->toPlainText();
    if (editPEquZ_->isOk())
        settings_->pEquationZ = editPEquZ_->toPlainText();

    // update widgets visibility

    const bool
            isFile = settings_->type ==
                    GEOM::GeometryFactorySettings::T_FILE,
            canTriangle = (settings_->type !=
                            GEOM::GeometryFactorySettings::T_GRID_XZ
                            && settings_->type !=
                            GEOM::GeometryFactorySettings::T_GRID),
            hasTriangle = (canTriangle && (settings_->asTriangles || isFile)),
            has2Segments = (settings_->type ==
                            GEOM::GeometryFactorySettings::T_UV_SPHERE
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_GRID_XZ
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_GRID
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_CYLINDER_CLOSED
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_CYLINDER_OPEN
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_TORUS),
            has3Segments = (has2Segments && settings_->type ==
                            GEOM::GeometryFactorySettings::T_GRID),
            hasSmallRadius = (settings_->type ==
                            GEOM::GeometryFactorySettings::T_TORUS);

    cbTriangles_->setVisible( canTriangle && !isFile);

    cbCalcNormals_->setVisible( hasTriangle && !settings_->convertToLines );
    cbCalcNormalsBeforePrimEqu_->setVisible(
                hasTriangle && settings_->transformPrimitivesWithEquation );
    cbConvertToLines_->setVisible( hasTriangle );
    spinTess_->setVisible( settings_->tesselate );

    labelNormAmt_->setVisible( settings_->normalizeVertices );
    spinNormAmt_->setVisible( settings_->normalizeVertices );

    editFilename_->setVisible(isFile);
    butLoadModelFile_->setVisible(isFile);
    cbSharedVert_->setVisible( !isFile ); // XXX remove when ObjLoader supports vertex sharing

    labelSeg_->setVisible( has2Segments );
    spinSegX_->setVisible( has2Segments );
    spinSegY_->setVisible( has2Segments );
    spinSegZ_->setVisible( has3Segments );

    labelSmallRadius_->setVisible( hasSmallRadius );
    spinSmallRadius_->setVisible( hasSmallRadius );

    spinRemoveProb_->setVisible( cbRemove_->isChecked() );
    spinRemoveSeed_->setVisible( cbRemove_->isChecked() );

    editEquX_->setVisible( settings_->transformWithEquation );
    editEquY_->setVisible( settings_->transformWithEquation );
    editEquZ_->setVisible( settings_->transformWithEquation );

    editPEquX_->setVisible( settings_->transformPrimitivesWithEquation );
    editPEquY_->setVisible( settings_->transformPrimitivesWithEquation );
    editPEquZ_->setVisible( settings_->transformPrimitivesWithEquation );

    updateGeometry_();
}

void GeometryDialog::updateWidgets_()
{
    ignoreUpdate_ = true;

    for (uint i=0; i<settings_->numTypes; ++i)
    {
        comboType_->addItem(settings_->typeNames[i], i);
        if (settings_->type == (GEOM::GeometryFactorySettings::Type)i)
            comboType_->setCurrentIndex(i);
    }
    editFilename_->setText(settings_->filename);
    cbTriangles_->setChecked(settings_->asTriangles);
    cbConvertToLines_->setChecked(settings_->convertToLines);
    cbSharedVert_->setChecked(settings_->sharedVertices);
    cbCalcNormals_->setChecked(settings_->calcNormals);
    cbNorm_->setChecked(settings_->normalizeVertices);
    spinNormAmt_->setValue(settings_->normalization);
    spinS_->setValue(settings_->scale);
    spinSX_->setValue(settings_->scaleX);
    spinSY_->setValue(settings_->scaleY);
    spinSZ_->setValue(settings_->scaleZ);
    spinSmallRadius_->setValue(settings_->smallRadius);
    spinSegX_->setValue(settings_->segmentsX);
    spinSegY_->setValue(settings_->segmentsY);
    spinSegZ_->setValue(settings_->segmentsZ);
    cbTess_->setChecked(settings_->tesselate);
    spinTess_->setValue(settings_->tessLevel);
    cbRemove_->setChecked(settings_->removeRandomly);
    spinRemoveProb_->setValue(settings_->removeProb);
    spinRemoveSeed_->setValue(settings_->removeSeed);
    cbTransformEqu_->setChecked(settings_->transformWithEquation);
    cbTransformPrimEqu_->setChecked(settings_->transformPrimitivesWithEquation);
    cbCalcNormalsBeforePrimEqu_->setChecked(settings_->calcNormalsBeforePrimitiveEquation);

    editEquX_->setPlainText(settings_->equationX);
    editEquY_->setPlainText(settings_->equationY);
    editEquZ_->setPlainText(settings_->equationZ);
    editPEquX_->setPlainText(settings_->pEquationX);
    editPEquY_->setPlainText(settings_->pEquationY);
    editPEquZ_->setPlainText(settings_->pEquationZ);

    ignoreUpdate_ = false;
}

void GeometryDialog::loadModelFile_()
{
    QString filename =
        IO::Files::getOpenFileName(IO::FT_MODEL, this);

    if (filename.isEmpty())
        return;

    editFilename_->setText(filename);
    updateFromWidgets_();
}

void GeometryDialog::updatePresetList_(const QString &selectFilename)
{
    ignoreUpdate_ = true;

    comboPreset_->clear();

    // search directory for preset files

    const QString path = IO::Files::directory(IO::FT_GEOMETRY_SETTINGS);
    QDir dir(path);
    QStringList filters;
    for (auto &ext : IO::fileTypeExtensions[IO::FT_GEOMETRY_SETTINGS])
        filters << ("*." + ext);
    QStringList names =
        dir.entryList(
                filters,
                QDir::Files | QDir::Readable | QDir::NoDotAndDotDot,
                QDir::Name | QDir::IgnoreCase | QDir::LocaleAware
                );

    // fill combo-box
    comboPreset_->addItem("-");

    int sel = -1;
    for (auto &n : names)
    {
        if (selectFilename == n)
            sel = comboPreset_->count();
        QString display = n;
        display.replace("."+IO::fileTypeExtensions[IO::FT_GEOMETRY_SETTINGS][0], "");
        comboPreset_->addItem(display, path + QDir::separator() + n);
    }

    // select desired
    if (sel>=0)
        comboPreset_->setCurrentIndex(sel);

    updatePresetButtons_();

    ignoreUpdate_ = false;
}

void GeometryDialog::updatePresetButtons_()
{
    const bool isPreset = comboPreset_->currentIndex() > 0;
    butSavePreset_->setEnabled( isPreset );
    butDeletePreset_->setEnabled( isPreset );
}

void GeometryDialog::savePreset_()
{
    int index = comboPreset_->currentIndex();
    if (index < 1)
        return;

    settings_->saveFile(comboPreset_->itemData(index).toString());
}


void GeometryDialog::savePresetAs_()
{
    QString filename =
            IO::Files::getSaveFileName(IO::FT_GEOMETRY_SETTINGS, this);

    if (filename.isEmpty())
        return;

    settings_->saveFile(filename);

    updatePresetList_(filename);
}

void GeometryDialog::deletePreset_()
{

}

void GeometryDialog::presetSelected_()
{
    if (ignoreUpdate_)
        return;

    updatePresetButtons_();

    const int index = comboPreset_->currentIndex();
    if (index < 0)
        return;

    const QString filename = comboPreset_->itemData(index).toString();

    GEOM::GeometryFactorySettings set;

    if (!filename.isEmpty())
        set.loadFile(filename);

    setGeometrySettings(set);
}

void GeometryDialog::setGeometrySettings(const GEOM::GeometryFactorySettings & s)
{
    *settings_ = s;

    updateWidgets_();

    updateGeometry_();
}


} // namespace GUI
} // namespace MO
