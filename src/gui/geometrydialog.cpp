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
#include <QMenu>
#include <QCloseEvent>

#include "geometrydialog.h"
#include "geom/geometry.h"
#include "geom/geometryfactorysettings.h"
#include "geom/geometrycreator.h"
#include "geom/geometrymodifierchain.h"
#include "geom/geometrymodifier.h"
#include "widget/geometrywidget.h"
#include "widget/doublespinbox.h"
#include "widget/spinbox.h"
#include "widget/geometrymodifierwidget.h"
#include "tool/stringmanip.h"
#include "widget/equationeditor.h"
#include "io/files.h"
#include "io/log.h"
#include "geometryexportdialog.h"

namespace MO {
namespace GUI {

GeometryDialog::GeometryDialog(const GEOM::GeometryFactorySettings *set,
                               QWidget *parent, Qt::WindowFlags flags) :
    QDialog         (parent, flags),
    settings_       (new GEOM::GeometryFactorySettings()),
    creator_        (0),
    geometry_       (0),
    updateGeometryLater_(false),
    ignoreUpdate_   (false),
    closeRequest_   (false)
{
    setObjectName("_GeometryWidget");
    setWindowTitle(tr("geometry editor"));

    setMinimumSize(960,600);

    if (set)
        *settings_ = *set;

    createMainWidgets_();

    modifiers_ = settings_->modifierChain;

    createModifierWidgets_();

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

void GeometryDialog::closeEvent(QCloseEvent * e)
{
    // keep result
    // (because QDialog::closeEvent() seems to reset it)
    int r = result();

    if (geoWidget_->isGlInitialized())
    {
        geoWidget_->shutDownGL();
        closeRequest_ = true;
        e->ignore();
    }
    else QDialog::closeEvent(e);

    setResult(r);
}

void GeometryDialog::onGlReleased()
{
    MO_DEBUG_GL("GeometryDialog::onGlReleased()");
    if (closeRequest_)
        close();
}

void GeometryDialog::createMainWidgets_()
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
                geoWidget_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                connect(geoWidget_, SIGNAL(glInitialized()), this, SLOT(updateFromWidgets_()));
                connect(geoWidget_, SIGNAL(glReleased()), this, SLOT(onGlReleased()));

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

                    cb = new QCheckBox(tr("bumpmapped"), this);
                    lh2->addWidget(cb);
                    cb->setChecked(geoWidget_->isShowNormalMap());
                    connect(cb, &QCheckBox::stateChanged, [this](int state)
                    {
                        geoWidget_->setShowNormalMap(state == Qt::Checked);
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

                    // shared vertices
                    cbSharedVert_ = new QCheckBox(tr("shared vertices"), this);
                    lh2->addWidget(cbSharedVert_);
                    cbSharedVert_->setChecked(settings_->sharedVertices);
                    connect(cbSharedVert_, SIGNAL(stateChanged(int)),
                            this, SLOT(updateFromWidgets_()));

                // small radius
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

                // color
                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    spinR_ = new DoubleSpinBox(this);
                    lh2->addWidget(spinR_);
                    spinR_->setStatusTip("Red amount of initital color");
                    spinR_->setDecimals(5);
                    spinR_->setSingleStep(0.1);
                    spinR_->setRange(0.0, 1);
                    spinR_->setValue(settings_->colorR);
                    QPalette pal(spinR_->palette());
                    pal.setColor(QPalette::Text, QColor(100,0,0));
                    spinR_->setPalette(pal);
                    connect(spinR_, SIGNAL(valueChanged(double)),
                            this, SLOT(updateFromWidgets_()));

                    spinG_ = new DoubleSpinBox(this);
                    lh2->addWidget(spinG_);
                    spinG_->setStatusTip("Green amount of initital color");
                    spinG_->setDecimals(5);
                    spinG_->setSingleStep(0.1);
                    spinG_->setRange(0.0, 1);
                    spinG_->setValue(settings_->colorG);
                    pal.setColor(QPalette::Text, QColor(0,70,0));
                    spinG_->setPalette(pal);
                    connect(spinG_, SIGNAL(valueChanged(double)),
                            this, SLOT(updateFromWidgets_()));

                    spinB_ = new DoubleSpinBox(this);
                    lh2->addWidget(spinB_);
                    spinB_->setStatusTip("Blue amount of initital color");
                    spinB_->setDecimals(5);
                    spinB_->setSingleStep(0.1);
                    spinB_->setRange(0.0, 1);
                    spinB_->setValue(settings_->colorB);
                    pal.setColor(QPalette::Text, QColor(0,0,140));
                    spinB_->setPalette(pal);
                    connect(spinB_, SIGNAL(valueChanged(double)),
                            this, SLOT(updateFromWidgets_()));

                    spinA_ = new DoubleSpinBox(this);
                    lh2->addWidget(spinA_);
                    spinA_->setStatusTip("Alpha amount of initital color");
                    spinA_->setDecimals(5);
                    spinA_->setSingleStep(0.1);
                    spinA_->setRange(0.0, 1);
                    spinA_->setValue(settings_->colorA);
                    connect(spinA_, SIGNAL(valueChanged(double)),
                            this, SLOT(updateFromWidgets_()));



                // add-new-modifier button
                auto but = new QPushButton(this);
                lv->addWidget(but);
                but->setText(tr("new modifier"));
                connect(but, &QPushButton::clicked, [=]()
                {
                    newModifierPopup_(0);
                });

                // place to add modifier widgets
                modifierLayout_ = new QVBoxLayout();
                lv->addLayout(modifierLayout_);
                modifierLayout_->setMargin(0);
                modifierLayout_->setSpacing(1);


                // info label
                labelInfo_ = new QLabel(this);
                lv->addWidget(labelInfo_);
                labelInfo_->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum);

                // OK/Cancel

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    but = new QPushButton(tr("Ok"), this);
                    lh2->addWidget(but);
                    but->setDefault(true);
                    connect(but, &QPushButton::clicked, [=]()
                    {
                        setResult(Accepted);
                        close();
                    });

                    but = new QPushButton(tr("Cancel"), this);
                    lh2->addWidget(but);
                    connect(but, &QPushButton::clicked, [=]()
                    {
                        setResult(Rejected);
                        close();
                    });

                // Export

                but = new QPushButton(tr("Export as file"), this);
                lv->addWidget(but);
                connect(but, &QPushButton::clicked, [=]()
                {
                    if (!geometry_)
                        return;

                    GeometryExportDialog exp;
                    exp.setGeometry(geometry_);
                    exp.exec();
                });

        statusBar_ = new QStatusBar(this);
        lv0->addWidget(statusBar_);

        progressBar_ = new QProgressBar(this);
        progressBar_->setOrientation(Qt::Horizontal);
        progressBar_->setRange(0,100);
        progressBar_->setVisible(false);
        statusBar_->addPermanentWidget(progressBar_);
}

void GeometryDialog::createModifierWidgets_()
{
    setUpdatesEnabled(false);

    // clear previous
    for (auto m : modifierWidgets_)
    {
        m->deleteLater();
    }
    modifierWidgets_.clear();

    // create widgets for geometry modifiers
    for (auto m : modifiers_->modifiers())
    {
        auto w = new GeometryModifierWidget(m, expandedModifiers_.contains(m), this);
        modifierLayout_->addWidget(w);
        modifierWidgets_.append(w);
        connect(w, SIGNAL(requestUp(GEOM::GeometryModifier*)),
                this, SLOT(modifierUp_(GEOM::GeometryModifier*)));
        connect(w, SIGNAL(requestDown(GEOM::GeometryModifier*)),
                this, SLOT(modifierDown_(GEOM::GeometryModifier*)));
        connect(w, SIGNAL(requestDelete(GEOM::GeometryModifier*)),
                this, SLOT(modifierDelete_(GEOM::GeometryModifier*)));
        connect(w, SIGNAL(requestInsertNew(GEOM::GeometryModifier*)),
                this, SLOT(newModifierPopup_(GEOM::GeometryModifier*)));
        connect(w, SIGNAL(requestMuteChange(GEOM::GeometryModifier*,bool)),
                this, SLOT(modifierMuteChange_(GEOM::GeometryModifier*,bool)));
        connect(w, SIGNAL(expandedChange(GEOM::GeometryModifier*,bool)),
                this, SLOT(modifierExpandedChanged_(GEOM::GeometryModifier*,bool)));
        connect(w, SIGNAL(valueChanged(GEOM::GeometryModifier*)),
                this, SLOT(updateFromWidgets_()));
    }

    // a "stretch" that can be deleted later
    auto stretch = new QWidget(this);
    modifierLayout_->addWidget(stretch);
    modifierLayout_->setStretch(modifierLayout_->indexOf(stretch), 2);
    modifierWidgets_.append(stretch);

    setUpdatesEnabled(true);
}

void GeometryDialog::modifierExpandedChanged_(GEOM::GeometryModifier * g, bool expanded)
{
    if (expanded)
        expandedModifiers_.insert(g);
    else
        expandedModifiers_.remove(g);
}

void GeometryDialog::modifierMuteChange_(GEOM::GeometryModifier * g, bool mute)
{
    g->setEnabled(!mute);
    updateFromWidgets_();
}

void GeometryDialog::modifierUp_(GEOM::GeometryModifier * g)
{
    if ( modifiers_->moveModifierUp(g) )
    {
        createModifierWidgets_();
        updateFromWidgets_();
    }
}

void GeometryDialog::modifierDown_(GEOM::GeometryModifier * g)
{
    if ( modifiers_->moveModifierDown(g) )
    {
        createModifierWidgets_();
        updateFromWidgets_();
    }
}

void GeometryDialog::modifierDelete_(GEOM::GeometryModifier * g)
{
    if ( modifiers_->deleteModifier(g) )
    {
        createModifierWidgets_();
        updateFromWidgets_();
    }
}

void GeometryDialog::newModifierPopup_(GEOM::GeometryModifier *before)
{
    QMenu * menu = new QMenu(this);
    const QList<QString> classnames = GEOM::GeometryModifierChain::modifierClassNames();
    const QList<QString> guinames = GEOM::GeometryModifierChain::modifierGuiNames();
    for (int i=0; i<classnames.size(); ++i)
    {
        QAction * a = new QAction(guinames[i], menu);
        a->setData(classnames[i]);
        menu->addAction(a);
    }

    connect(menu, &QMenu::triggered, [=](QAction * a)
    {
        const QString classname = a->data().toString();
        if (!before)
            modifiers_->addModifier(classname);
        else
            modifiers_->insertModifier(classname, before);

        createModifierWidgets_();
        updateFromWidgets_();
    });

    connect(menu, SIGNAL(aboutToHide()), menu, SLOT(deleteLater()));

    menu->popup(QCursor::pos());
}

void GeometryDialog::changeView_()
{
    if (comboView_->currentIndex() < 0)
        return;

    GeometryWidget::RenderMode rm = (GeometryWidget::RenderMode)
            comboView_->itemData(comboView_->currentIndex()).toInt();

    geoWidget_->setRenderMode(rm);
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

    geometry_ = g;
    geoWidget_->setGeometry(g);

    progressBar_->setVisible(false);
    labelInfo_->setText(tr("%1 vertices, %2 triangles, %3 lines, memory: %4")
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
    settings_->asTriangles = cbTriangles_->isChecked();
    settings_->sharedVertices = cbSharedVert_->isChecked();
    settings_->smallRadius = spinSmallRadius_->value();
    settings_->segmentsX = spinSegX_->value();
    settings_->segmentsY = spinSegY_->value();
    settings_->segmentsZ = spinSegZ_->value();
    settings_->colorR = spinR_->value();
    settings_->colorG = spinG_->value();
    settings_->colorB = spinB_->value();
    settings_->colorA = spinA_->value();

    // update widgets visibility

    const bool
            isFile = settings_->type ==
                    GEOM::GeometryFactorySettings::T_FILE,
            canOnlyTriangle = isFile
                    || settings_->type == GEOM::GeometryFactorySettings::T_BOX_UV,
            canTriangle = (settings_->type !=
                            GEOM::GeometryFactorySettings::T_GRID_XZ
                            && settings_->type !=
                            GEOM::GeometryFactorySettings::T_LINE_GRID),
//            hasTriangle = (canTriangle && (settings_->asTriangles || isFile)),
            has2Segments = (settings_->type ==
                            GEOM::GeometryFactorySettings::T_UV_SPHERE
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_GRID_XZ
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_LINE_GRID
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_CYLINDER_CLOSED
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_CYLINDER_OPEN
                           || settings_->type ==
                            GEOM::GeometryFactorySettings::T_TORUS),
            has3Segments = (has2Segments && settings_->type ==
                            GEOM::GeometryFactorySettings::T_LINE_GRID),
            hasSmallRadius = (settings_->type ==
                            GEOM::GeometryFactorySettings::T_TORUS);

    editFilename_->setVisible(isFile);
    butLoadModelFile_->setVisible(isFile);

    labelSeg_->setVisible( has2Segments );
    spinSegX_->setVisible( has2Segments );
    spinSegY_->setVisible( has2Segments );
    spinSegZ_->setVisible( has3Segments );

    labelSmallRadius_->setVisible( hasSmallRadius );
    spinSmallRadius_->setVisible( hasSmallRadius );

    cbTriangles_->setVisible( canTriangle && !canOnlyTriangle);

    updateGeometry_();
}

void GeometryDialog::updateWidgets_()
{
    ignoreUpdate_ = true;

    comboType_->clear();
    for (uint i=0; i<settings_->numTypes; ++i)
    {
        comboType_->addItem(settings_->typeNames[i], i);
        if (settings_->type == (GEOM::GeometryFactorySettings::Type)i)
            comboType_->setCurrentIndex(i);
    }
    editFilename_->setText(settings_->filename);
    cbTriangles_->setChecked(settings_->asTriangles);
    cbSharedVert_->setChecked(settings_->sharedVertices);
    spinSmallRadius_->setValue(settings_->smallRadius);
    spinSegX_->setValue(settings_->segmentsX);
    spinSegY_->setValue(settings_->segmentsY);
    spinSegZ_->setValue(settings_->segmentsZ);
    spinR_->setValue(settings_->colorR);
    spinG_->setValue(settings_->colorG);
    spinB_->setValue(settings_->colorB);
    spinA_->setValue(settings_->colorA);

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
    modifiers_ = settings_->modifierChain;

    updateWidgets_();
    createModifierWidgets_();
    updateGeometry_();
}


} // namespace GUI
} // namespace MO
