/** @file keepmodulatordialog.h

    @brief Dialog for selecting modulators to be reused after pasting

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 15.10.2014</p>
*/

#ifndef MOSRC_GUI_KEEPMODULATORDIALOG_H
#define MOSRC_GUI_KEEPMODULATORDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSet>

class QListWidget;

namespace MO {

class Scene;
class Object;
class Parameter;

namespace GUI { class KeepModulatorDialog; }


/** Structure for tracking modulators */
class KeepModulators
{
    friend class GUI::KeepModulatorDialog;
public:

    /** Constructs the class for the Scene.
        The modulators present in @p scene must not change until the call to createNewModulators()!
        The objects added with addNewObject() must exist in the supplied Scene object! */
    KeepModulators(Scene * scene);
    ~KeepModulators();

    /** Add the object from the clipboard here, before adding to Scene.
        The object and all of it's childs are saved (if they are modulators in Scene). */
    void addOriginalObject(Object *);

    /** Add the object after it has been added to the Scene (with new idNames).
        The object and all of it's childs are checked for modulator reuse. */
    void addNewObject(Object *);

    /** Returns true when modulators where present in addOriginalObject().
        On true, you should run the KeepModulatorDialog,
        otherwise it can be ignored. */
    bool modulatorsPresent() const;

    /** Creates the new modulation paths.
        For every (sub-)object added with addOriginalObject() that is a wired modulator
        to some Parameter in Scene, the new objects added with addNewObject() are
        made to modulate the same goal. */
    void createNewModulators();

private:

    struct Private;
    Private * p_;
};



namespace GUI {

/** Dialog to expose KeepModulators to the user */
class KeepModulatorDialog : public QDialog
{
    Q_OBJECT
public:

    /** Creates a Dialog to select which modulators to reuse.
        The @p mods parameter must be initialized before.
        The user can select to reuse all or certain modulators and
        these new modulation paths are created on closing of the dialog. */
    explicit KeepModulatorDialog(KeepModulators&, QWidget *parent = 0);

    ~KeepModulatorDialog();

private slots:

    void doit_();
    void selectAll_();
    void unselectAll_();
    void flipAll_();
private:

    void createWidgets_();
    void createList_();

    KeepModulators & mods_;

    QListWidget * list_;
};

} // namespace GUI
} // namespace MO

#endif // MOSRC_GUI_KEEPMODULATORDIALOG_H
