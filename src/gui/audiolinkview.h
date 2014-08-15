/** @file audiolinkview.h

    @brief Editor for AudioUnits and Modulators

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef MOSRC_GUI_AUDIOLINKVIEW_H
#define MOSRC_GUI_AUDIOLINKVIEW_H

#include <QWidget>
#include <QMap>

#include "object/object_fwd.h"

class QGridLayout;

namespace MO {
namespace GUI {

class AudioUnitWidget;

class AudioLinkView : public QWidget
{
    Q_OBJECT
public:
    explicit AudioLinkView(QWidget *parent = 0);

    void setScene(Scene * );

signals:

public slots:

private:

    void createMainWidgets_();
    void clearAudioUnitWidgets_();

    void createAudioUnitWidgets_();
    void createAudioUnitWidgetsRec_(const QList<Object*>& objects, int& row, int col);

    QGridLayout * grid_;

    Scene * scene_;
    //QList<AudioUnit*> topLevelUnits_;

    QMap<QString, AudioUnitWidget*> unitWidgets_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_AUDIOLINKVIEW_H
