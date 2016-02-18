/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/17/2016</p>
*/

#ifndef MOSRC_GUI_EVOLUTIONBASEVIEWER_H
#define MOSRC_GUI_EVOLUTIONBASEVIEWER_H

#include <QDialog>

class QImage;
class QLabel;

namespace MO {
class EvolutionBase;
namespace GUI {


/** Simple window that displays renders an EvolutionBase */
class EvolutionBaseViewer : public QDialog
{
    Q_OBJECT
public:
    explicit EvolutionBaseViewer(QWidget *parent = 0);
    ~EvolutionBaseViewer();

    EvolutionBase* specimen() const { return evo_; }

    /** Installs a copy as renderer. Set to zero to clear. */
    void setSpecimen(const EvolutionBase*);

public slots:
    void saveJson();
    void saveJson(const QString& fn);
    void saveImage();
    void saveImage(const QString& fn);

protected:
    void mousePressEvent(QMouseEvent*) override;
    void paintEvent(QPaintEvent*) override;

private:
    EvolutionBase* evo_;
    QImage* image_;
    QLabel* label_;
    bool doRender_;
};

} // namespace GUI
} // namespace MO


#endif // EVOLUTIONBASEVIEWER_H
