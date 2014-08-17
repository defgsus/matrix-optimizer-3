/** @file audiounitwidget.h

    @brief Widget for displaying/connecting AudioUnits

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef AUDIOUNITWIDGET_H
#define AUDIOUNITWIDGET_H

#include <QWidget>
#include <QStringList>

#include "object/object_fwd.h"

namespace MO {
namespace GUI {

class AudioUnitConnectorWidget;

class AudioUnitWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AudioUnitWidget(AudioUnit * au, QWidget *parent = 0);

    AudioUnit * audioUnit() const { return unit_; }

    /** Returns read access to the list of direct children AudioUnit IDs */
    const QStringList& connectedIds() const { return connectedIds_; }

    /** Returns read access to input connector widgets */
    const QList<AudioUnitConnectorWidget*>& audioInWidgets() const { return audioIns_; }

    /** Returns read access to output connector widgets */
    const QList<AudioUnitConnectorWidget*>& audioOutWidgets() const { return audioOuts_; }

    /** Returns read access to modulator output connector widgets */
    const QList<AudioUnitConnectorWidget*>& modulatorOutWidgets() const { return modulatorOuts_; }

signals:

public slots:

    /** Updates the ModulatorObjectFloat value displays */
    void updateValueOutputs();

    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

private:

    void createWidgets_();
    QWidget * createHeader_();

    AudioUnit * unit_;

    QStringList connectedIds_;

    QList<AudioUnitConnectorWidget*>
        audioIns_,
        audioOuts_,
        modulatorOuts_;
};


} // namespace GUI
} // namespace MO

#endif // AUDIOUNITWIDGET_H
