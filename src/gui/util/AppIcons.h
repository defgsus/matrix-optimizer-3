/** @file appicons.h

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 12.01.2015</p>
*/

#ifndef MOSRC_GUI_UTIL_APPICONS_H
#define MOSRC_GUI_UTIL_APPICONS_H

#include <QSize>

class QIcon;
class QColor;

namespace MO {

class Object;

class AppIcons
{
public:

    /** Sets the default color for the next icons */
    static void setDefaultColor(const QColor& color);
    /** Sets the default size of the next icons */
    static void setDefaultSize(const QSize& s);

    /** Returns an icon for the MO::Object::Type enum, with default settings.
        Object must be valid, of course. */
    static QIcon iconForType(int object_type);

    /** Returns an icon for the object type, with default settings.
        Object must be valid, of course. */
    static QIcon iconForObject(const Object * o);

    /** Returns an icon for the object type, overriding default settings.
        Object must be valid, of course. */
    static QIcon iconForObject(const Object * o, const QColor& color, const QSize& size = QSize());

private:

    AppIcons() { }

    struct Private;
};

} // namespace MO

#endif // MOSRC_GUI_UTIL_APPICONS_H
