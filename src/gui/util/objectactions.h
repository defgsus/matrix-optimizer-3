/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/14/2016</p>
*/

#ifndef MOSRC_GUI_UTIL_OBJECTACTIONS_H
#define MOSRC_GUI_UTIL_OBJECTACTIONS_H

#include <functional>

#include <QCoreApplication> // for tr()

#include "object/object_fwd.h"
#include "tool/actionlist.h"
#include "gl/opengl_fwd.h"

namespace MO {
namespace GUI {

class ObjectActions
{
    Q_DECLARE_TR_FUNCTIONS(ObjectActions)
public:

    // --- high level menus ---

    static void createEditActions(
            ActionList& actions, Object*, QObject* parent = nullptr);

    static void createNewObjectActions(
            ActionList& actions, Object * obj,
            QObject* parent = nullptr,
            std::function<void(Object*)> newObjectFunc = nullptr);


    // --- low level bits ----

    static void createSaveTextureMenu(
            ActionList& actions, Object*, QObject* parent = nullptr);

    static QMenu * createObjectsMenu(
            Object *parent, bool with_template, bool with_shortcuts,
            bool child_only, int groups, QObject* pparent);

    // ---- helper ----

    static void saveTexture(const GL::Texture* tex);
};


} // namespace GUI
} // namespace MO


#endif // MOSRC_GUI_UTIL_OBJECTACTIONS_H
