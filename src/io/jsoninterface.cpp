/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/19/2016</p>
*/

#include <cstddef>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonArray>
#include <QFile>

#include "jsoninterface.h"
#include "io/error.h"

namespace MO {

QString JsonInterface::toJsonString() const
{
    QJsonDocument doc(toJson());
    return QString::fromUtf8(doc.toJson());
}

void JsonInterface::fromJsonString(const QString &jsonString)
{
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(jsonString.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError)
        MO_IO_ERROR(PARSE, "Error parsing json string: "
                      << error.errorString().toStdString());
    auto main = doc.object();
    fromJson(main);
}


void JsonInterface::saveJsonFile(const QString& filename) const
{
    QString js = toJsonString();

    QFile file(filename);
    if (!file.open(QFile::WriteOnly))
        MO_IO_ERROR(WRITE, "Could not open '"
                      << filename.toStdString() << "' for writing,\n"
                      << file.errorString().toStdString());

    file.write(js.toUtf8());
}

void JsonInterface::loadJsonFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        MO_IO_ERROR(READ, "Could not open '"
                      << filename.toStdString() << "' for reading,\n"
                      << file.errorString().toStdString());

    auto js = QString::fromUtf8(file.readAll());
    file.close();

    fromJsonString(js);
}



// ---------------------------- HELPER -------------------------------

namespace
{
    /** Converter for arguments acceptable by QJsonValue constructor */
    template <typename T>
    struct JsonValueTraits
    {
        static QJsonValue to(T v) { return QJsonValue(v); }
    };

    template <>
    struct JsonValueTraits<size_t>
    {
        static QJsonValue to(size_t v) { return QJsonValue((qint64)v); }
    };
}




const char* JsonInterface::JSON::typeName(const QJsonValue& v)
{
    switch (v.type())
    {
        case QJsonValue::Null: return "NULL";
        case QJsonValue::Bool: return "bool";
        case QJsonValue::Double: return "double";
        case QJsonValue::String: return "string";
        case QJsonValue::Array: return "array";
        case QJsonValue::Object: return "object";
        case QJsonValue::Undefined: return "undefined";
    }
    return "*undefined*";
}

template <>
double JsonInterface::JSON::expect(const QJsonValue& v)
{
    if (!v.isDouble())
        MO_IO_ERROR(PARSE, "Expected double value, got " << typeName(v));
    return v.toDouble();
}

template <>
int JsonInterface::JSON::expect(const QJsonValue& v)
{
    if (!v.isDouble() && !v.isString())
        MO_IO_ERROR(PARSE, "Expected int value, got " << typeName(v));
    if (v.isString())
    {
        bool ok;
        int k = v.toString().toInt(&ok);
        if (!ok)
            MO_IO_ERROR(PARSE, "Expected int value, got non-int string '"
                        << v.toString() << "'");
        return k;
    }
    return v.toInt();
}

template <>
float JsonInterface::JSON::expect(const QJsonValue& v)
{
    return expect<double>(v);
}

template <>
size_t JsonInterface::JSON::expect(const QJsonValue& v)
{
    return expect<int>(v);
}



QJsonArray JsonInterface::JSON::expectArray(const QJsonValue& v)
{
    if (!v.isArray())
        MO_IO_ERROR(PARSE, "Expected json array, got " << typeName(v));
    return v.toArray();
}

QJsonValue JsonInterface::JSON::expectChild(
        const QJsonObject& parent, const QString& key)
{
    if (!parent.contains(key))
        MO_IO_ERROR(PARSE, "Expected '" << key << "' object, not found");
    return parent.value(key);
}









template <typename T>
QJsonArray JsonInterface::JSON::toArray(const std::vector<T>& data)
{
    QJsonArray a;
    for (T v : data)
        a.append(JsonValueTraits<T>::to(v));
    return a;
}

template <typename T>
void JsonInterface::JSON::fromArray(std::vector<T>& dst, const QJsonArray& src)
{
    dst.resize(src.size());
    for (size_t i=0; i<dst.size(); ++i)
        dst[i] = expect<T>(src.at(i));
}

template <typename T>
void JsonInterface::JSON::fromArray(std::vector<T>& dst, const QJsonValue& src)
{
    fromArray(dst, expectArray(src));
}

// --- template instantiation ---

#define MO__INSTANTIATE(type__) \
template QJsonArray JsonInterface::JSON::toArray<type__>( const std::vector<type__>&); \
template void JsonInterface::JSON::fromArray<type__>(std::vector<type__>& dst, const QJsonArray& src); \
template void JsonInterface::JSON::fromArray<type__>(std::vector<type__>& dst, const QJsonValue& src);

//template type__ JsonInterface::JSON::expect(const QJsonValue&);

MO__INSTANTIATE(int)
MO__INSTANTIATE(float)
MO__INSTANTIATE(double)
MO__INSTANTIATE(size_t)


#undef MO__INSTANTIATE

} // namespace MO






