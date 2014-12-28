/** @file geometrymodifierwidget.cpp

    @brief Widget for GeometryModifier classes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/14/2014</p>
*/

#include <QLayout>
#include <QLabel>
#include <QToolButton>
#include <QIcon>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

#include "geometrymodifierwidget.h"
#include "io/error.h"
#include "io/files.h"
#include "doublespinbox.h"
#include "spinbox.h"
#include "groupwidget.h"
#include "equationeditor.h"
#include "geom/geometry.h"
#include "geom/geometrymodifierscale.h"
#include "geom/geometrymodifiertesselate.h"
#include "geom/geometrymodifiertranslate.h"
#include "geom/geometrymodifierrotate.h"
#include "geom/geometrymodifiernormals.h"
#include "geom/geometrymodifiernormalize.h"
#include "geom/geometrymodifierconvertlines.h"
#include "geom/geometrymodifierremove.h"
#include "geom/geometrymodifiervertexgroup.h"
#include "geom/geometrymodifiervertexequation.h"
#include "geom/geometrymodifierprimitiveequation.h"
#include "geom/geometrymodifierextrude.h"
#include "geom/geometrymodifiertexcoords.h"
#include "geom/geometrymodifierduplicate.h"
#include "geom/geometrymodifiercreate.h"
#ifndef MO_DISABLE_ANGELSCRIPT
#include "geom/geometrymodifierangelscript.h"
#include "script/angelscript_geometry.h"
#include "angelscriptwidget.h"
#endif

namespace MO {
namespace GUI {

GeometryModifierWidget::GeometryModifierWidget(GEOM::GeometryModifier * geom, bool expanded, QWidget *parent) :
    QWidget                 (parent),
    modifier_               (geom),
    funcUpdateFromWidgets_  (0),
    funcUpdateWidgets_      (0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    createWidgets_(expanded);

    updateWidgetValues();
}


void GeometryModifierWidget::createWidgets_(bool expanded)
{
    auto l0 = new QVBoxLayout(this);
    l0->setMargin(0);
    group_ = new GroupWidget(modifier_->guiName(), expanded, this);
    l0->addWidget(group_);

    group_->setStatusTip(modifier_->statusTip());
    group_->setHeaderStatusTip(modifier_->statusTip());

    auto butMute = new QToolButton(this);
    group_->addHeaderWidget(butMute);
    butMute->setText(tr("M"));
    butMute->setStatusTip(tr("Enables or disables the modifier while keeping it in the list"));
    butMute->setFixedSize(20,20);
    butMute->setCheckable(true);
    butMute->setChecked(!modifier_->isEnabled());
    connect(butMute, &QToolButton::clicked, [=]()
    {
        emit requestMuteChange(modifier_, butMute->isChecked());
    });

    group_->addHeaderSpacing(5);

    auto butUp = new QToolButton(this);
    group_->addHeaderWidget(butUp);
    butUp->setArrowType(Qt::UpArrow);
    butUp->setFixedSize(20,20);
    butUp->setStatusTip(tr("Moves the modifier up in the list"));
    connect(butUp, &QToolButton::clicked, [=](){ emit requestUp(modifier_); });

    auto butDown = new QToolButton(this);
    group_->addHeaderWidget(butDown);
    butDown->setArrowType(Qt::DownArrow);
    butDown->setFixedSize(20,20);
    butDown->setStatusTip(tr("Moves the modifier down in the list"));
    connect(butDown, &QToolButton::clicked, [=](){ emit requestDown(modifier_); });

    auto butInsert = new QToolButton(this);
    group_->addHeaderWidget(butInsert);
    butInsert->setIcon(QIcon(":/icon/new_letters.png"));
    butInsert->setFixedSize(20,20);
    butInsert->setStatusTip(tr("Creates a new modifier above this one"));
    connect(butInsert, &QToolButton::clicked, [=](){ emit requestInsertNew(modifier_); });

    group_->addHeaderSpacing(5);

    auto butRemove = new QToolButton(this);
    group_->addHeaderWidget(butRemove);
    butRemove->setIcon(QIcon(":/icon/delete.png"));
    butRemove->setFixedSize(20,20);
    butRemove->setStatusTip(tr("Permanently deletes the modifier"));
    connect(butRemove, &QToolButton::clicked, [=](){ emit requestDelete(modifier_); });

    connect(group_, &GroupWidget::expanded, [=]()
    {
        emit expandedChange(modifier_, true);
    });
    connect(group_, &GroupWidget::collapsed, [=]()
    {
        emit expandedChange(modifier_, false);
    });


#define MO__DOUBLESPIN(var__, statustip__, step__, min__, max__) \
    auto var__ = new DoubleSpinBox(this);           \
    group_->addWidget(var__);                       \
    var__->setStatusTip(statustip__);               \
    var__->setDecimals(5);                          \
    var__->setSingleStep(step__);                   \
    var__->setRange(min__, max__);                  \
    connect(var__, SIGNAL(valueChanged(double)),    \
            this, SLOT(updateFromWidgets_()));

#define MO__SPIN(var__, statustip__, min__, max__)  \
    auto var__ = new SpinBox(this);                 \
    group_->addWidget(var__);                       \
    var__->setStatusTip(statustip__);               \
    var__->setRange(min__, max__);                  \
    connect(var__, SIGNAL(valueChanged(int)),       \
            this, SLOT(updateFromWidgets_()));

#define MO__CHECKBOX(var__, text__, statusTip__, value__)   \
    auto var__ = new QCheckBox(this);                       \
    group_->addWidget(var__);                               \
    var__->setText(text__);                                 \
    var__->setStatusTip(statusTip__);                       \
    var__->setChecked(value__);                             \
    connect(var__, SIGNAL(stateChanged(int)),               \
            this, SLOT(updateFromWidgets_()));

    // -------- create widgets for specific modifier class -----------

    if (auto creator = dynamic_cast<GEOM::GeometryModifierCreate*>(modifier_))
    {
        createCreatorWidgets_(creator);
    }

    if (auto scale = dynamic_cast<GEOM::GeometryModifierScale*>(modifier_))
    {
        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            auto label = new QLabel(tr("overall scale"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinall, tr("Overall scale of the geometry"), 0.1, 0.0001, 1000000.0);
            lh->addWidget(spinall);

        lh = new QHBoxLayout();
        group_->addLayout(lh);

            MO__DOUBLESPIN(spinx, tr("X-scale of the the geometry"), 0.1, 0.0001, 1000000.0);
            lh->addWidget(spinx);

            MO__DOUBLESPIN(spiny, tr("Y-scale of the the geometry"), 0.1, 0.0001, 1000000.0);
            lh->addWidget(spiny);

            MO__DOUBLESPIN(spinz, tr("Z-scale of the the geometry"), 0.1, 0.0001, 1000000.0);
            lh->addWidget(spinz);

        funcUpdateFromWidgets_ = [=]()
        {
            scale->setScaleAll(spinall->value());
            scale->setScaleX(spinx->value());
            scale->setScaleY(spiny->value());
            scale->setScaleZ(spinz->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinall->setValue(scale->getScaleAll());
            spinx->setValue(scale->getScaleX());
            spiny->setValue(scale->getScaleY());
            spinz->setValue(scale->getScaleZ());
        };
    }



    if (auto rot = dynamic_cast<GEOM::GeometryModifierRotate*>(modifier_))
    {
        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            auto label = new QLabel(tr("angle °"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinangle, tr("Angle of rotation in degree"), 1, -1000000.0, 1000000.0);
            lh->addWidget(spinangle);

        lh = new QHBoxLayout();
        group_->addLayout(lh);


            MO__DOUBLESPIN(spinx, tr("X-axis of rotation"), 1, -1000000.0, 1000000.0);
            lh->addWidget(spinx);

            MO__DOUBLESPIN(spiny, tr("Y-axis of rotation"), 1, -1000000.0, 1000000.0);
            lh->addWidget(spiny);

            MO__DOUBLESPIN(spinz, tr("Z-axis of rotation"), 1, -1000000.0, 1000000.0);
            lh->addWidget(spinz);

        funcUpdateFromWidgets_ = [=]()
        {
            rot->setRotationAngle(spinangle->value());
            rot->setRotationX(spinx->value());
            rot->setRotationY(spiny->value());
            rot->setRotationZ(spinz->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinangle->setValue(rot->getRotationAngle());
            spinx->setValue(rot->getRotationX());
            spiny->setValue(rot->getRotationY());
            spinz->setValue(rot->getRotationZ());
        };
    }


    if (auto trans = dynamic_cast<GEOM::GeometryModifierTranslate*>(modifier_))
    {
        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            MO__DOUBLESPIN(spinx, tr("Offset of translation on x-axis"), 0.1, -1000000.0, 1000000.0);
            lh->addWidget(spinx);

            MO__DOUBLESPIN(spiny, tr("Offset of translation on y-axis"), 0.1, -1000000.0, 1000000.0);
            lh->addWidget(spiny);

            MO__DOUBLESPIN(spinz, tr("Offset of translation on z-axis"), 0.1, -1000000.0, 1000000.0);
            lh->addWidget(spinz);

        funcUpdateFromWidgets_ = [=]()
        {
            trans->setTranslationX(spinx->value());
            trans->setTranslationY(spiny->value());
            trans->setTranslationZ(spinz->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinx->setValue(trans->getTranslationX());
            spiny->setValue(trans->getTranslationY());
            spinz->setValue(trans->getTranslationZ());
        };
    }



    if (auto normals = dynamic_cast<GEOM::GeometryModifierNormals*>(modifier_))
    {
        MO__CHECKBOX(cbcalc, tr("calculate normals"),
                     tr("Automatically calculates the normals for the triangles"),
                     normals->getCalcNormals());

        MO__CHECKBOX(cbinvert, tr("invert normals"),
                     tr("Inverts normals, so that they point into the opposite direction"),
                     normals->getInvertNormals());

        funcUpdateFromWidgets_ = [=]()
        {
            normals->setCalcNormals(cbcalc->isChecked());
            normals->setInvertNormals(cbinvert->isChecked());
        };

        funcUpdateWidgets_ = [=]()
        {
            cbcalc->setChecked(normals->getCalcNormals());
            cbinvert->setChecked(normals->getInvertNormals());
        };
    }




    if (auto normalz = dynamic_cast<GEOM::GeometryModifierNormalize*>(modifier_))
    {
        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            auto label = new QLabel(tr("normalization"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinmix, tr("Amount of normalization between 0 and 1"),
                           0.05, 0, 1);
            lh->addWidget(spinmix);

        funcUpdateFromWidgets_ = [=]()
        {
            normalz->setNormalization(spinmix->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinmix->setValue(normalz->getNormalization());
        };
    }

    if (auto tex = dynamic_cast<GEOM::GeometryModifierTexCoords*>(modifier_))
    {
        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            auto label = new QLabel(tr("offset"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinox, tr("Offset on x-axis"),
                           0.01, -1, 1);
            lh->addWidget(spinox);

            MO__DOUBLESPIN(spinoy, tr("Offset on y-axis"),
                           0.01, -1, 1);
            lh->addWidget(spinoy);

        lh = new QHBoxLayout();
        group_->addLayout(lh);

            label = new QLabel(tr("scale"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinx, tr("Scale on x-axis"),
                           0.1, -1000000, 1000000);
            lh->addWidget(spinx);

            MO__DOUBLESPIN(spiny, tr("Scale on y-axis"),
                           0.1, -1000000, 1000000);
            lh->addWidget(spiny);


        lh = new QHBoxLayout();
        group_->addLayout(lh);

            MO__CHECKBOX(cbinvx, tr("invert x"),
                         tr("Inverts the texture coordinates on the x-axis"),
                         tex->getInvertX());
            lh->addWidget(cbinvx);

            MO__CHECKBOX(cbinvy, tr("invert y"),
                         tr("Inverts the texture coordinates on the y-axis"),
                         tex->getInvertY());
            lh->addWidget(cbinvy);

        funcUpdateFromWidgets_ = [=]()
        {
            tex->setOffsetX(spinox->value());
            tex->setOffsetY(spinoy->value());
            tex->setScaleX(spinx->value());
            tex->setScaleY(spiny->value());
            tex->setInvertX(cbinvx->isChecked());
            tex->setInvertY(cbinvy->isChecked());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinox->setValue(tex->getOffsetX());
            spinoy->setValue(tex->getOffsetY());
            spinx->setValue(tex->getScaleX());
            spiny->setValue(tex->getScaleY());
            cbinvx->setChecked(tex->getInvertX());
            cbinvy->setChecked(tex->getInvertY());
        };
    }


    if (auto tess = dynamic_cast<GEOM::GeometryModifierTesselate*>(modifier_))
    {
        MO__SPIN(spinlevel, tr("Order of tesselation. Be careful, this is an exponential value!"),
                 1, 10);

        funcUpdateFromWidgets_ = [=]()
        {
            tess->setTesselationLevel(spinlevel->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinlevel->setValue(tess->getTesselationLevel());
        };
    }



    if (auto remove = dynamic_cast<GEOM::GeometryModifierRemove*>(modifier_))
    {
        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            auto label = new QLabel(tr("random probability"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinprob, tr("Probability of removing a primitive, between 0 and 1"),
                           0.05, 0, 1);
            lh->addWidget(spinprob);

        lh = new QHBoxLayout();
        group_->addLayout(lh);

            label = new QLabel(tr("random seed"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__SPIN(spinseed, tr("Random seed which determines the pattern of removal"),
                           0, 10000000);
            lh->addWidget(spinseed);

        funcUpdateFromWidgets_ = [=]()
        {
            remove->setRemoveProbability(spinprob->value());
            remove->setRandomSeed(spinseed->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinprob->setValue(remove->getRemoveProbability());
            spinseed->setValue(remove->getRandomSeed());
        };
    }



    if (/*auto conv = */dynamic_cast<GEOM::GeometryModifierConvertLines*>(modifier_))
    {
        funcUpdateFromWidgets_ = [=]()
        {
        };

        funcUpdateWidgets_ = [=]()
        {
        };
    }



    if (auto shared = dynamic_cast<GEOM::GeometryModifierVertexGroup*>(modifier_))
    {
        MO__CHECKBOX(cbshared, tr("share vertices"),
                     tr("If enabled, primitives will shared common vertices, "
                        "if disabled, all vertices will be made unique."),
                     shared->getShared());

        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            auto label = new QLabel(tr("vertex distance"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinthresh,
                     tr("Maximum distance in which vertices will be considered 'the same'"),
                     0.001, GEOM::Geometry::minimumThreshold, 1000);
            lh->addWidget(spinthresh);

        funcUpdateFromWidgets_ = [=]()
        {
            shared->setShared(cbshared->isChecked());
            shared->setThreshold(spinthresh->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            cbshared->setChecked(shared->getShared());
            spinthresh->setValue(shared->getThreshold());
        };
    }



    if (auto extrude = dynamic_cast<GEOM::GeometryModifierExtrude*>(modifier_))
    {
        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            auto label = new QLabel(tr("constant"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinconst, tr("Extrudes triangles along their normal by a constant value"),
                           0.05, -1000000, 1000000);
            lh->addWidget(spinconst);

        lh = new QHBoxLayout();
        group_->addLayout(lh);

            label = new QLabel(tr("factor"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinfac, tr("Extrudes triangles vertices along their normal by a factor "
                                         "of the length of adjecent edges"),
                           0.05, -1000000, 1000000);
            lh->addWidget(spinfac);

        lh = new QHBoxLayout();
        group_->addLayout(lh);

            label = new QLabel(tr("shift"), this);
            group_->addWidget(label);
            lh->addWidget(label);

            MO__DOUBLESPIN(spinshift, tr("Shifts the extruded vertices towards the center of the triangles"),
                           0.05, -1000000, 1000000);
            lh->addWidget(spinshift);

        MO__CHECKBOX(cbFaces, tr("create orthogonal faces"),
                     tr("Enables the creation of the outside faces, "
                        "orthogonal to extruded faces"),
                     extrude->getDoOuterFaces());

        MO__CHECKBOX(cbQuads, tr("recognize edges"),
                     tr("If e.g. two triangles are recognized as forming a quad, "
                        "no orthogonal face is created for the inner edge"),
                     extrude->getDoRecognizeEdges());

        funcUpdateFromWidgets_ = [=]()
        {
            extrude->setConstant(spinconst->value());
            extrude->setFactor(spinfac->value());
            extrude->setShiftCenter(spinshift->value());
            extrude->setDoOuterFaces(cbFaces->isChecked());
            extrude->setDoRecognizeEdges(cbQuads->isChecked());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinconst->setValue(extrude->getConstant());
            spinfac->setValue(extrude->getFactor());
            spinshift->setValue(extrude->getShiftCenter());
            cbFaces->setChecked(extrude->getDoOuterFaces());
            cbQuads->setChecked(extrude->getDoRecognizeEdges());
        };
    }



    if (auto equ = dynamic_cast<GEOM::GeometryModifierVertexEquation*>(modifier_))
    {
        QStringList vars = {
            "x", "y", "z", "i", "s", "t",
            "red", "green", "blue", "alpha", "bright" };

        auto editEqu = new EquationEditor(this);
        group_->addWidget(editEqu);
        editEqu->addVariables(vars);
        editEqu->setPlainText(equ->getEquation());
        connect(editEqu, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        funcUpdateFromWidgets_ = [=]()
        {
            equ->setEquation(editEqu->toPlainText());
        };

        funcUpdateWidgets_ = [=]()
        {
            editEqu->setPlainText(equ->getEquation());
        };
    }




    if (auto equ = dynamic_cast<GEOM::GeometryModifierPrimitiveEquation*>(modifier_))
    {
        QStringList vars = {
            "x", "y", "z", "nx", "ny", "nz", "s", "t", "i", "p",
            "red", "green", "blue", "alpha", "bright",
            "x1", "y1", "z1", "x2", "y2", "z2", "x3", "y3", "z3",
            "nx1", "ny1", "nz1", "nx2", "ny2", "nz2", "nx3", "ny3", "nz3",
            "s1", "t1", "s2", "t2", "s3", "t3",
            "red1", "green1", "blue1", "alpha1",
            "red2", "green2", "blue2", "alpha2",
            "red3", "green3", "blue3", "alpha3" };

        auto editEqu = new EquationEditor(this);
        group_->addWidget(editEqu);
        editEqu->addVariables(vars);
        editEqu->setPlainText(equ->getEquation());
        connect(editEqu, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        funcUpdateFromWidgets_ = [=]()
        {
            equ->setEquation(editEqu->toPlainText());
        };

        funcUpdateWidgets_ = [=]()
        {
            editEqu->setPlainText(equ->getEquation());
        };
    }



    if (auto dupli = dynamic_cast<GEOM::GeometryModifierDuplicate*>(modifier_))
    {
        auto label = new QLabel(tr("number duplicates"), this);
        group_->addWidget(label);

        auto lh = new QHBoxLayout();
        group_->addLayout(lh);

            MO__SPIN(spinx, tr("Number of duplicates on x axis"),
                           1, 1000000);
            lh->addWidget(spinx);

            MO__SPIN(spiny, tr("Number of duplicates on y axis"),
                           1, 1000000);
            lh->addWidget(spiny);

            MO__SPIN(spinz, tr("Number of duplicates on z axis"),
                           1, 1000000);
            lh->addWidget(spinz);

        QStringList vars = {
            "x", "y", "z", "i", "s", "t",
            "red", "green", "blue", "alpha",
            "d", "dx", "dy", "dz" };

        auto editEqu = new EquationEditor(this);
        group_->addWidget(editEqu);
        editEqu->addVariables(vars);
        editEqu->setPlainText(dupli->getEquation());
        connect(editEqu, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        funcUpdateFromWidgets_ = [=]()
        {
            dupli->setDuplicatesX(spinx->value());
            dupli->setDuplicatesY(spiny->value());
            dupli->setDuplicatesZ(spinz->value());
            dupli->setEquation(editEqu->toPlainText());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinx->setValue(dupli->duplicatesX());
            spiny->setValue(dupli->duplicatesY());
            spinz->setValue(dupli->duplicatesZ());
            editEqu->setPlainText(dupli->getEquation());
        };
    }

#ifndef MO_DISABLE_ANGELSCRIPT
    if (auto script = dynamic_cast<GEOM::GeometryModifierAngelScript*>(modifier_))
    {
        auto edit = new AngelScriptWidget(this);
        //edit->setMinimumHeight(500);
        group_->addWidget(edit);
        edit->setScriptEngine( GeometryEngineAS::createNullEngine() );
        connect(edit, SIGNAL(scriptTextChanged()), this, SLOT(updateFromWidgets_()));

        funcUpdateFromWidgets_ = [=]()
        {
            script->setScript(edit->scriptText());
        };

        funcUpdateWidgets_ = [=]()
        {
            edit->setScriptText(script->script());
        };
    }
#endif

}

void GeometryModifierWidget::createCreatorWidgets_(GEOM::GeometryModifierCreate * settings)
{
    // geometry type
    auto comboType = new QComboBox(this);
    group_->addWidget(comboType);
    comboType->setStatusTip("Selects the type of geometry");

    for (uint i=0; i<settings->numTypes; ++i)
    {
        comboType->addItem(settings->typeNames[i], i);
        if (settings->type() == (GEOM::GeometryModifierCreate::Type)i)
            comboType->setCurrentIndex(i);
    }

    connect(comboType, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(updateFromWidgets_()));

    auto lh2 = new QHBoxLayout();
    group_->addLayout(lh2);

        // filename
        auto editFilename = new QLineEdit(this);
        group_->addWidget(editFilename);
        lh2->addWidget(editFilename);
        editFilename->setText(settings->filename());
        editFilename->setReadOnly(true);

        auto butLoadModelFile = new QToolButton(this);
        group_->addWidget(butLoadModelFile);
        lh2->addWidget(butLoadModelFile);
        butLoadModelFile->setText("...");
        connect(butLoadModelFile, &QToolButton::clicked, [=]()
        {
            QString filename =
                IO::Files::getOpenFileName(IO::FT_MODEL, this);

            if (filename.isEmpty())
                return;

            editFilename->setText(filename);
            updateFromWidgets_();
        });


    lh2 = new QHBoxLayout();
    group_->addLayout(lh2);

        // create triangles
        auto cbTriangles = new QCheckBox(tr("create triangles"), this);
        group_->addWidget(cbTriangles);
        lh2->addWidget(cbTriangles);
        cbTriangles->setChecked(settings->asTriangles());
        connect(cbTriangles, SIGNAL(stateChanged(int)),
                this, SLOT(updateFromWidgets_()));

        // shared vertices
        auto cbSharedVert = new QCheckBox(tr("shared vertices"), this);
        group_->addWidget(cbSharedVert);
        lh2->addWidget(cbSharedVert);
        cbSharedVert->setChecked(settings->sharedVertices());
        connect(cbSharedVert, SIGNAL(stateChanged(int)),
                this, SLOT(updateFromWidgets_()));

    // small radius
    lh2 = new QHBoxLayout();
    group_->addLayout(lh2);

        auto labelSmallRadius = new QLabel(tr("small radius"), this);
        group_->addWidget(labelSmallRadius);
        lh2->addWidget(labelSmallRadius);

        auto spinSmallRadius = new DoubleSpinBox(this);
        group_->addWidget(spinSmallRadius);
        lh2->addWidget(spinSmallRadius);
        spinSmallRadius->setStatusTip("Smaller radius");
        spinSmallRadius->setDecimals(5);
        spinSmallRadius->setSingleStep(0.02);
        spinSmallRadius->setRange(0.0001, 100000);
        spinSmallRadius->setValue(settings->smallRadius());
        connect(spinSmallRadius, SIGNAL(valueChanged(double)),
                this, SLOT(updateFromWidgets_()));

    // segments
    auto labelSeg = new QLabel(tr("segments"), this);
    group_->addWidget(labelSeg);
    group_->addWidget(labelSeg);

    lh2 = new QHBoxLayout();
    group_->addLayout(lh2);

        auto spinSegX = new SpinBox(this);
        group_->addWidget(spinSegX);
        lh2->addWidget(spinSegX);
        spinSegX->setStatusTip("Number of segments (X)");
        spinSegX->setRange(1, 10000);
        spinSegX->setValue(settings->segmentsX());
        connect(spinSegX, SIGNAL(valueChanged(int)),
                this, SLOT(updateFromWidgets_()));

        auto spinSegY = new SpinBox(this);
        group_->addWidget(spinSegY);
        lh2->addWidget(spinSegY);
        spinSegY->setStatusTip("Number of segments (Y)");
        spinSegY->setRange(1, 10000);
        spinSegY->setValue(settings->segmentsY());
        connect(spinSegY, SIGNAL(valueChanged(int)),
                this, SLOT(updateFromWidgets_()));

        auto spinSegZ = new SpinBox(this);
        group_->addWidget(spinSegZ);
        lh2->addWidget(spinSegZ);
        spinSegZ->setStatusTip("Number of segments (Z)");
        spinSegZ->setRange(0, 10000);
        spinSegZ->setValue(settings->segmentsZ());
        connect(spinSegZ, SIGNAL(valueChanged(int)),
                this, SLOT(updateFromWidgets_()));

    // color
    lh2 = new QHBoxLayout();
    group_->addLayout(lh2);

        auto spinR = new DoubleSpinBox(this);
        group_->addWidget(spinR);
        lh2->addWidget(spinR);
        spinR->setStatusTip("Red amount of initital color");
        spinR->setDecimals(5);
        spinR->setSingleStep(0.1);
        spinR->setRange(0.0, 1);
        spinR->setValue(settings->red());
        QPalette pal(spinR->palette());
        pal.setColor(QPalette::Text, QColor(100,0,0));
        spinR->setPalette(pal);
        connect(spinR, SIGNAL(valueChanged(double)),
                this, SLOT(updateFromWidgets_()));

        auto spinG = new DoubleSpinBox(this);
        group_->addWidget(spinG);
        lh2->addWidget(spinG);
        spinG->setStatusTip("Green amount of initital color");
        spinG->setDecimals(5);
        spinG->setSingleStep(0.1);
        spinG->setRange(0.0, 1);
        spinG->setValue(settings->green());
        pal.setColor(QPalette::Text, QColor(0,70,0));
        spinG->setPalette(pal);
        connect(spinG, SIGNAL(valueChanged(double)),
                this, SLOT(updateFromWidgets_()));

        auto spinB = new DoubleSpinBox(this);
        group_->addWidget(spinB);
        lh2->addWidget(spinB);
        spinB->setStatusTip("Blue amount of initital color");
        spinB->setDecimals(5);
        spinB->setSingleStep(0.1);
        spinB->setRange(0.0, 1);
        spinB->setValue(settings->blue());
        pal.setColor(QPalette::Text, QColor(0,0,140));
        spinB->setPalette(pal);
        connect(spinB, SIGNAL(valueChanged(double)),
                this, SLOT(updateFromWidgets_()));

        auto spinA = new DoubleSpinBox(this);
        group_->addWidget(spinA);
        lh2->addWidget(spinA);
        spinA->setStatusTip("Alpha amount of initital color");
        spinA->setDecimals(5);
        spinA->setSingleStep(0.1);
        spinA->setRange(0.0, 1);
        spinA->setValue(settings->alpha());
        connect(spinA, SIGNAL(valueChanged(double)),
                this, SLOT(updateFromWidgets_()));

    auto funcUpdateVisibility = [=]()
    {
        const bool
                isFile = settings->type() ==
                        GEOM::GeometryModifierCreate::T_FILE,
                canOnlyTriangle = isFile
                        || settings->type() == GEOM::GeometryModifierCreate::T_BOX_UV,
                canTriangle = (settings->type() !=
                                GEOM::GeometryModifierCreate::T_GRID_XZ
                                && settings->type() !=
                                GEOM::GeometryModifierCreate::T_LINE_GRID),
    //            hasTriangle = (canTriangle && (settings_->asTriangles || isFile)),
                has2Segments = (settings->type() ==
                                GEOM::GeometryModifierCreate::T_UV_SPHERE
                               || settings->type() ==
                                GEOM::GeometryModifierCreate::T_GRID_XZ
                               || settings->type() ==
                                GEOM::GeometryModifierCreate::T_LINE_GRID
                               || settings->type() ==
                                GEOM::GeometryModifierCreate::T_CYLINDER_CLOSED
                               || settings->type() ==
                                GEOM::GeometryModifierCreate::T_CYLINDER_OPEN
                               || settings->type() ==
                                GEOM::GeometryModifierCreate::T_TORUS),
                has3Segments = (has2Segments && settings->type() ==
                                GEOM::GeometryModifierCreate::T_LINE_GRID),
                hasSmallRadius = (settings->type() ==
                                GEOM::GeometryModifierCreate::T_TORUS);

        group_->setVisible(editFilename, isFile);
        group_->setVisible(butLoadModelFile, isFile);

        group_->setVisible(labelSeg, has2Segments );
        group_->setVisible(spinSegX, has2Segments );
        group_->setVisible(spinSegY, has2Segments );
        group_->setVisible(spinSegZ, has3Segments );

        group_->setVisible(labelSmallRadius, hasSmallRadius );
        group_->setVisible(spinSmallRadius, hasSmallRadius );

        group_->setVisible(cbTriangles, canTriangle && !canOnlyTriangle);
    };

    //bool ignoreUpdates = false;

    funcUpdateFromWidgets_ = [=]()
    {
        //if (ignoreUpdates)
        //    return;

        settings->setAsTriangles(cbTriangles->isChecked());
        settings->setSharedVertices(cbSharedVert->isChecked());
        settings->setRed(spinR->value());
        settings->setGreen(spinG->value());
        settings->setBlue(spinB->value());
        settings->setAlpha(spinA->value());
        settings->setSegmentsX(spinSegX->value());
        settings->setSegmentsY(spinSegY->value());
        settings->setSegmentsZ(spinSegZ->value());
        settings->setFilename(editFilename->text());
        settings->setSmallRadius(spinSmallRadius->value());
        if (comboType->currentIndex() >= 0)
            settings->setType((GEOM::GeometryModifierCreate::Type)
                comboType->itemData(comboType->currentIndex()).toInt());

        funcUpdateVisibility();
    };

    funcUpdateWidgets_ = [=]()
    {
        funcUpdateVisibility();

        //ignoreUpdates = true;

        editFilename->setText(settings->filename());
        cbTriangles->setChecked(settings->asTriangles());
        cbSharedVert->setChecked(settings->sharedVertices());
        spinSmallRadius->setValue(settings->smallRadius());
        spinSegX->setValue(settings->segmentsX());
        spinSegY->setValue(settings->segmentsY());
        spinSegZ->setValue(settings->segmentsZ());
        spinR->setValue(settings->red());
        spinG->setValue(settings->green());
        spinB->setValue(settings->blue());
        spinA->setValue(settings->alpha());

        //ignoreUpdates = false;
    };

}


void GeometryModifierWidget::updateWidgetValues()
{
    MO_ASSERT(funcUpdateWidgets_, "no update function defined");

    if (funcUpdateWidgets_)
        funcUpdateWidgets_();

}

void GeometryModifierWidget::updateFromWidgets_()
{
    MO_ASSERT(funcUpdateFromWidgets_, "no update function defined");

    if (funcUpdateFromWidgets_)
        funcUpdateFromWidgets_();

    emit valueChanged(modifier_);
}


} // namespace GUI
} // namespace MO
