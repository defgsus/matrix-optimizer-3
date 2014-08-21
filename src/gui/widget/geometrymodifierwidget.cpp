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
#include "geom/geometrymodifiertexcoordequation.h"
#include "geom/geometrymodifierextrude.h"
#include "geom/geometrymodifiertexcoords.h"

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

    auto butMute = new QToolButton(this);
    group_->addHeaderWidget(butMute);
    butMute->setText(tr("M"));
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
    connect(butUp, &QToolButton::clicked, [=](){ emit requestUp(modifier_); });

    auto butDown = new QToolButton(this);
    group_->addHeaderWidget(butDown);
    butDown->setArrowType(Qt::DownArrow);
    butDown->setFixedSize(20,20);
    connect(butDown, &QToolButton::clicked, [=](){ emit requestDown(modifier_); });

    auto butInsert = new QToolButton(this);
    group_->addHeaderWidget(butInsert);
    butInsert->setIcon(QIcon(":/icon/new_letters.png"));
    butInsert->setFixedSize(20,20);
    connect(butInsert, &QToolButton::clicked, [=](){ emit requestInsertNew(modifier_); });

    group_->addHeaderSpacing(5);

    auto butRemove = new QToolButton(this);
    group_->addHeaderWidget(butRemove);
    butRemove->setIcon(QIcon(":/icon/delete.png"));
    butRemove->setFixedSize(20,20);
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

        funcUpdateFromWidgets_ = [=]()
        {
            extrude->setConstant(spinconst->value());
            extrude->setFactor(spinfac->value());
        };

        funcUpdateWidgets_ = [=]()
        {
            spinconst->setValue(extrude->getConstant());
            spinfac->setValue(extrude->getFactor());
        };
    }



    if (auto equ = dynamic_cast<GEOM::GeometryModifierVertexEquation*>(modifier_))
    {
        QStringList vars = { "x", "y", "z", "i" };

        auto editEquX = new EquationEditor(this);
        group_->addWidget(editEquX);
        editEquX->addVariables(vars);
        editEquX->setPlainText(equ->getEquationX());
        connect(editEquX, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        auto editEquY = new EquationEditor(this);
        group_->addWidget(editEquY);
        editEquY->addVariables(vars);
        editEquY->setPlainText(equ->getEquationY());
        connect(editEquY, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        auto editEquZ = new EquationEditor(this);
        group_->addWidget(editEquZ);
        editEquZ->addVariables(vars);
        editEquZ->setPlainText(equ->getEquationZ());
        connect(editEquZ, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        funcUpdateFromWidgets_ = [=]()
        {
            equ->setEquationX(editEquX->toPlainText());
            equ->setEquationY(editEquY->toPlainText());
            equ->setEquationZ(editEquZ->toPlainText());
        };

        funcUpdateWidgets_ = [=]()
        {
            editEquX->setPlainText(equ->getEquationX());
            editEquY->setPlainText(equ->getEquationY());
            editEquZ->setPlainText(equ->getEquationZ());
        };
    }




    if (auto equ = dynamic_cast<GEOM::GeometryModifierPrimitiveEquation*>(modifier_))
    {
        QStringList vars = {
            "x", "y", "z", "nx", "ny", "nz", "i", "p",
            "x1", "y1", "z1", "x2", "y2", "z2", "x3", "y3", "z3",
            "nx1", "ny1", "nz1", "nx2", "ny2", "nz2", "nx3", "ny3", "nz3" };

        auto editEquX = new EquationEditor(this);
        group_->addWidget(editEquX);
        editEquX->addVariables(vars);
        editEquX->setPlainText(equ->getEquationX());
        connect(editEquX, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        auto editEquY = new EquationEditor(this);
        group_->addWidget(editEquY);
        editEquY->addVariables(vars);
        editEquY->setPlainText(equ->getEquationY());
        connect(editEquY, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        auto editEquZ = new EquationEditor(this);
        group_->addWidget(editEquZ);
        editEquZ->addVariables(vars);
        editEquZ->setPlainText(equ->getEquationZ());
        connect(editEquZ, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        funcUpdateFromWidgets_ = [=]()
        {
            equ->setEquationX(editEquX->toPlainText());
            equ->setEquationY(editEquY->toPlainText());
            equ->setEquationZ(editEquZ->toPlainText());
        };

        funcUpdateWidgets_ = [=]()
        {
            editEquX->setPlainText(equ->getEquationX());
            editEquY->setPlainText(equ->getEquationY());
            editEquZ->setPlainText(equ->getEquationZ());
        };
    }



    if (auto equ = dynamic_cast<GEOM::GeometryModifierTexCoordEquation*>(modifier_))
    {
        QStringList vars = { "x", "y", "z", "i", "s", "t" };

        auto editEquS = new EquationEditor(this);
        group_->addWidget(editEquS);
        editEquS->addVariables(vars);
        editEquS->setPlainText(equ->getEquationS());
        connect(editEquS, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        auto editEquT = new EquationEditor(this);
        group_->addWidget(editEquT);
        editEquT->addVariables(vars);
        editEquT->setPlainText(equ->getEquationT());
        connect(editEquT, SIGNAL(equationChanged()), this, SLOT(updateFromWidgets_()));

        funcUpdateFromWidgets_ = [=]()
        {
            equ->setEquationS(editEquS->toPlainText());
            equ->setEquationT(editEquT->toPlainText());
        };

        funcUpdateWidgets_ = [=]()
        {
            editEquS->setPlainText(equ->getEquationS());
            editEquT->setPlainText(equ->getEquationT());
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
