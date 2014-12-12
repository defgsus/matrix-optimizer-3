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
#include "io/settings.h"
#include "geometryexportdialog.h"
#include "gl/lightsettings.h"

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
    settings->restoreGeometry(this);

    if (set)
        *settings_ = *set;

    createMainWidgets_();

    createModifierWidgets_();

    updatePresetList_();

//    geomChanged_ = true;
}

GeometryDialog::~GeometryDialog()
{
    settings->storeGeometry(this);
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

                // --- view settings ---

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

                    auto tbut = new QToolButton(this);
                    lh2->addWidget(tbut);
                    tbut->setIcon(QIcon(":/icon/view_front.png"));
                    connect(tbut, &QToolButton::clicked,
                            [=]{ setViewDirection(Basic3DWidget::VD_FRONT); });

                    tbut = new QToolButton(this);
                    lh2->addWidget(tbut);
                    tbut->setIcon(QIcon(":/icon/view_back.png"));
                    connect(tbut, &QToolButton::clicked,
                            [=]{ setViewDirection(Basic3DWidget::VD_BACK); });

                    tbut = new QToolButton(this);
                    lh2->addWidget(tbut);
                    tbut->setIcon(QIcon(":/icon/view_left.png"));
                    connect(tbut, &QToolButton::clicked,
                            [=]{ setViewDirection(Basic3DWidget::VD_LEFT); });

                    tbut = new QToolButton(this);
                    lh2->addWidget(tbut);
                    tbut->setIcon(QIcon(":/icon/view_right.png"));
                    connect(tbut, &QToolButton::clicked,
                            [=]{ setViewDirection(Basic3DWidget::VD_RIGHT); });

                    tbut = new QToolButton(this);
                    lh2->addWidget(tbut);
                    tbut->setIcon(QIcon(":/icon/view_top.png"));
                    connect(tbut, &QToolButton::clicked,
                            [=]{ setViewDirection(Basic3DWidget::VD_TOP); });

                    tbut = new QToolButton(this);
                    lh2->addWidget(tbut);
                    tbut->setIcon(QIcon(":/icon/view_bottom.png"));
                    connect(tbut, &QToolButton::clicked,
                            [=]{ setViewDirection(Basic3DWidget::VD_BOTTOM); });

                lh2 = new QHBoxLayout();
                lv->addLayout(lh2);

                    auto cb = new QCheckBox(tr("coordinates"), this);
                    lh2->addWidget(cb);
                    cb->setChecked(geoWidget_->isShowGrid());
                    connect(cb, &QCheckBox::stateChanged, [this](int state)
                    {
                        geoWidget_->setShowGrid(state == Qt::Checked);
                    });

                    cb = new QCheckBox(tr("lights"), this);
                    lh2->addWidget(cb);
                    cb->setChecked(geoWidget_->isShowLights());
                    connect(cb, &QCheckBox::stateChanged, [this](int state)
                    {
                        geoWidget_->setShowLights(state == Qt::Checked);
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
    for (auto m : settings_->modifierChain()->modifiers())
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

void GeometryDialog::setViewDirection(int dir)
{
    Float distance = 5.f;

    if (geometry_)
    {
        Vec3 mi,ma;
        geometry_->getExtent(&ma, &ma);
        distance = std::max(distance,
                            std::max(std::max(ma[0], ma[1]), std::max(ma[2],
                std::max(std::max(std::abs(mi[0]),std::abs(mi[1])),std::abs(mi[2])))));
    }

    geoWidget_->viewSet((Basic3DWidget::ViewDirection)dir, distance);
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
    if ( settings_->modifierChain()->moveModifierUp(g) )
    {
        createModifierWidgets_();
        updateFromWidgets_();
    }
}

void GeometryDialog::modifierDown_(GEOM::GeometryModifier * g)
{
    if ( settings_->modifierChain()->moveModifierDown(g) )
    {
        createModifierWidgets_();
        updateFromWidgets_();
    }
}

void GeometryDialog::modifierDelete_(GEOM::GeometryModifier * g)
{
    if ( settings_->modifierChain()->deleteModifier(g) )
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
            settings_->modifierChain()->addModifier(classname);
        else
            settings_->modifierChain()->insertModifier(classname, before);

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
    connect(creator_, SIGNAL(succeeded()), this, SLOT(creationFinished_()));
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

    updateGeometry_();
}

void GeometryDialog::updateWidgets_()
{
    ignoreUpdate_ = true;

    ignoreUpdate_ = false;
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
    createModifierWidgets_();
    updateGeometry_();
}


} // namespace GUI
} // namespace MO
