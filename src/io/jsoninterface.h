/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/19/2016</p>
*/

#ifndef MOSRC_IO_JSONINTERFACE_H
#define MOSRC_IO_JSONINTERFACE_H


class QJsonObject;
class QJsonArray;
class QJsonValue;
class QString;

namespace MO {

/** Interface for loading/saving json data.
    Users of the interface need to implement the
    toJson() and fromJson() methods.
    This enables converting to/from json strings
    and file io.
*/
class JsonInterface
{
public:
    virtual ~JsonInterface() { }

    // ----------- pure methods --------------

    /** Return a QJsonObject with all data inside.
        Should throw a descriptive IoException on any errors. */
    virtual QJsonObject toJson() const = 0;

    /** Initializes the object from the QJsonObject.
        Should throw a descriptive IoException on severe errors. */
    virtual void fromJson(const QJsonObject&) = 0;

    // ------------ convenience --------------

    /** Converts this object's data to a Json string.
        Uses toJson().
        @throws IoException on any error */
    virtual QString toJsonString() const;

    /** Initializes this objects's data from a json string.
        Uses fromJson().
        @throws IoException on any error */
    virtual void fromJsonString(const QString&);

    /** Stores the json string to a file.
        Uses toJsonString().
        @throws IoException on any error */
    virtual void saveJsonFile(const QString& filename) const;

    /** Initializes this object's data from a json file.
        Uses fromJsonString().
        @throws IoException on any error */
    virtual void loadJsonFile(const QString& filename);

    /** namespace for static helper functions */
    struct JSON
    {
        static const char* typeName(const QJsonValue&);

        template <typename T>
        static T expect(const QJsonValue&);

        static QJsonValue expectChild(const QJsonObject& parent, const QString& key);
        static QJsonArray expectArray(const QJsonValue&);

        template <typename T>
        static QJsonArray toArray(const std::vector<T>&);

        template <typename T>
        static void fromArray(std::vector<T>& dst, const QJsonArray& src);

        template <typename T>
        static void fromArray(std::vector<T>& dst, const QJsonValue& src);
    };
};

} // namespace MO

#endif // MOSRC_IO_JSONINTERFACE_H
