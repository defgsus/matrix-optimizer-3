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

#include "geometrymodifierwidget.h"
#include "io/error.h"
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

namespace MO {
namespace GUI {

GeometryModifierWidget::GeometryModifierWidget(GEOM::GeometryModifier * geom, bool expanded, QWidget *parent) :
    QWidget                 (parent),
    modifier_               (geom),
    funcUpdateFromWidgets_  (0),
    funcUpdateWidgets_      (0)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

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
            extrude->setDoOuterFaces(cbFaces->isChecked());
            extrude->setDoRecognizeEdges(cbQuads->isChecked());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinconst->setValue(extrude->getConstant());
            spinfac->setValue(extrude->getFactor());
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
