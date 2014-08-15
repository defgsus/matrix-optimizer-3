/** @file audiolinkwindow.h

    @brief High-level editor for Scenes

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 8/15/2014</p>
*/

#ifndef MOSRC_GUI_AUDIOLINKWINDOW_H
#define MOSRC_GUI_AUDIOLINKWINDOW_H

#include <QMainWindow>

#include "object/object_fwd.h"

namespace MO {
namespace GUI {

class MainWindow;
class AudioLinkView;

class AudioLinkWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit AudioLinkWindow(QWidget *parent = 0);

    void setMainWindow(MainWindow*);
    void setScene(Scene *);
signals:

public slots:

private:

    void createWidgets_();

    Scene * scene_;

    MainWindow * mainWindow_;
    AudioLinkView * audioView_;
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_AUDIOLINKWINDOW_H
