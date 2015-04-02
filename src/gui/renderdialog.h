/** @file renderdialog.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/25/2015</p>
*/

#ifndef MOSRC_GUI_RENDERDIALOG_H
#define MOSRC_GUI_RENDERDIALOG_H

#include <QDialog>

namespace MO {
class Scene;
namespace GUI {

/** Dialog style window with complete disk render control */
class RenderDialog : public QDialog
{
    Q_OBJECT
public:
    explicit RenderDialog(const QString& sceneFilename, QWidget *parent = 0);
    ~RenderDialog();

    bool isRendering() const;

signals:

public slots:

    void startRender();
    void stopRender();

private slots:

    void p_onWidget_();
    void p_onUnitChange_(int idx);
    void p_onFinished_();
    void p_shutDown_();

protected:

    void closeEvent(QEvent*);
    void error(const QString&);

private:

    struct Private;
    Private * p_;
};


} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_RENDERDIALOG_H
