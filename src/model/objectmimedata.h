/** @file objectmimedata.h

    @brief

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 03.12.2014</p>
*/

#ifndef MOSRC_MODEL_OBJECTMIMEDATA_H
#define MOSRC_MODEL_OBJECTMIMEDATA_H

#include <QMimeData>

namespace MO {

class Object;

class ObjectDescription
{
public:

    /** A NULL pointer constructs an invalid description */
    ObjectDescription(Object *);

    bool isValid() const { return p_ptr_; }
    bool isFromSameApplicationInstance() const;

    Object * pointer() const { return static_cast<Object*>(p_ptr_); }
    quint64 type() const { return p_type_; }
    const QString& className() const { return p_class_; }
    const QString& name() const { return p_name_; }
    const QString& id() const { return p_id_; }

    QByteArray toByteArray() const;
    static ObjectDescription fromByteArray(const QByteArray &);


private:
    quint64 p_type_;
    QString
        p_class_,
        p_id_,
        p_name_;
    void * p_ptr_, * p_app_;
};

class ObjectMimeData : public QMimeData
{
    Q_OBJECT
public:

    static const QString mimeTypeString;

    // ----------- restore ----------------

    /** If this returns false, getObjectPointer() is pretty useless */
    bool isFromSameApplicationInstance() const
        { return getDescription().isFromSameApplicationInstance(); }

    /** Returns the stored description.
        Returns invalid description if none was stored. */
    ObjectDescription getDescription() const;

    // ------------ store --------------

    /** Sets the mime data */
    void setObject(Object *);

};

} // namespace MO

#endif // MOSRC_MODEL_OBJECTMIMEDATA_H
