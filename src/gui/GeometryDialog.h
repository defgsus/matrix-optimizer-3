/** @file geometrydialog.h

    @brief Editor for Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/28/2014</p>
*/

#ifndef MOSRC_GUI_GEOMETRYDIALOG_H
#define MOSRC_GUI_GEOMETRYDIALOG_H

#include <QDialog>
#include <QSet>

#include "gl/opengl_fwd.h"

class QComboBox;
class QCheckBox;
class QLabel;
class QLineEdit;
class QToolButton;
class QStatusBar;
class QProgressBar;
class QVBoxLayout;
class QScrollArea;

namespace MO {
class GeometryEditInterface;
namespace GUI {

class SpinBox;
class DoubleSpinBox;
class GeometryWidget;

class GeometryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GeometryDialog(const GEOM::GeometryFactorySettings * = 0,
                            QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~GeometryDialog();

    /** Returns the settings as edited in the dialog */
    const GEOM::GeometryFactorySettings& getGeometrySettings() const { return *settings_; }

    /** Creates and returns a Geometry edit dialog for the given object.
        The dialog is attached and reused and killed properly. */
    static GeometryDialog* openForInterface(GeometryEditInterface*);

signals:

    /** Emitted on accept or apply */
    void apply();

public slots:

    /* Sets the settings to display/edit */
    void setGeometrySettings(const GEOM::GeometryFactorySettings&);

    /** One of Basic3DWidget::ViewDirection enums */
    void setViewDirection(int);

    /** Save the current settings under the given filename */
    void saveGeometrySettings(const QString& filename);

protected slots:

    void changeView_();
    void updateGeometry_();
    void onChanged_();
    void updateFromWidgets_();
    void savePreset_();
    void savePresetAs_();
    void deletePreset_();
    void presetSelected_();

    void modifierUp_(GEOM::GeometryModifier*);
    void modifierDown_(GEOM::GeometryModifier*);
    void modifierDelete_(GEOM::GeometryModifier*);
    void modifierExpandedChanged_(GEOM::GeometryModifier*, bool expanded);
    void modifierMuteChange_(GEOM::GeometryModifier*, bool mute);
    void newModifierPopup_(GEOM::GeometryModifier * before);

    void creatorProgress_(int);
    void creationFailed_(const QString&);
    void creationFinished_();

    void onGlReleased();

    void setChanged_(bool c);
    bool isSaveToDiscard_() const;

protected:
    //void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    bool event(QEvent *);

    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent*);

    void createMainWidgets_();
    void createModifierWidgets_();
    void updateWidgets_();
    void updatePresetList_();
    void updatePresetButtons_();

    GeometryWidget * geoWidget_;
    GEOM::GeometryFactorySettings * settings_;
    GEOM::GeometryCreator * creator_;
    GEOM::Geometry * geometry_;
    bool updateGeometryLater_,
         ignoreUpdate_,
         closeRequest_,
         isChanged_,
         isAttached_;
    QString curPresetFile_;

    QList<QWidget*> modifierWidgets_;
    QVBoxLayout * modifierLayout_;
    QSet<GEOM::GeometryModifier*> expandedModifiers_;

    QTimer * updateTimer_;
    QStatusBar * statusBar_;
    QProgressBar * progressBar_;
    QLabel * labelInfo_;
    QToolButton *butSavePreset_, *butSavePresetAs_, *butDeletePreset_;
    QComboBox * comboPreset_, *comboView_, * comboType_;
    QScrollArea * geomScroll_;
    bool firstInit_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_GEOMETRYDIALOG_H
