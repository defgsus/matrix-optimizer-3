/** @file io.h

    @brief Text/XML Serializer and deserializer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 6/25/2014</p>
*/

#ifndef MOSCR_IO_XMLSTREAM_H
#define MOSCR_IO_XMLSTREAM_H

#include <string>
#include <vector>

#include <QString>
#include <QDateTime>

class QXmlStreamWriter;
class QXmlStreamReader;

namespace MO {
namespace IO {

template <class T>
struct PairStruct
{
    PairStruct(const QString& key, const T value)
        :   key(key), value(value) { }

    const QString& key;
    const T value;
};

template <class T>
PairStruct<T> Pair(const QString& key, const T value)
{
    return PairStruct<T>(key, value);
}


/** @brief serializer and deserializer
    <p>The backend is QXmlStream, so this is actually
    a convenience wrapper around it.</p>
*/
class XmlStream
{
public:

    // ------------------ ctor ----------------------

    XmlStream();
    ~XmlStream();

    // ------------------- io -----------------------

    void save(const QString& filename);
    void load(const QString& filename);

    /** Returns the current data stream */
    const QString data() const { return data_; }

    void setData(const QString& s) { data_ = s; }
    void setData(const QByteArray& a) { data_ = a; }

    // --------------- streaming --------------------

    /** Starts writing an xml.
        The main section will be set to @p main_section.
        Always call stopWriting() when you're finished */
    void startWriting(const QString& main_section = "mo-xml");
    void stopWriting();
    bool isWriteable();

    /** Starts reading an xml.
        The main section is expected to be @p main_section.
        An IoException is thrown if this is not the case.
        The current section will be @p main_section after the call.
        Always call stopReading() when you're finished. */
    void startReading(const QString& main_section = "mo-xml");
    void stopReading();
    bool isReadable();

    // --------------- verification -----------------

    /** @throws IoError when current section is not @p name */
    void verifySection(const QString& name);

    // ------------------ sections ------------------

    /** WRITE, Create a new (sub-)section. */
    void newSection(const QString& name);

    /** WRITE, Ends the current sub-section and goes to containing section. */
    void endSection();

    /** READ/WRITE, Returns if the current section matches @p name. */
    bool isSection(const QString& name) const;

    /** READ/WRITE, Returns name of the current section. */
    const QString& section() const;

    /** READ, open next sub-section.
        Returns true when a new subsection was found, false otherwise. */
    bool nextSubSection();

    /** READ, Leaves current (sub-)section.
        If there is no higher section, this does nothing. */
    void leaveSection();

    // ----------------- data write -----------------

    void write(const QString& key, const char * v) { write(key, QString(v)); }
    void write(const QString& key, const std::string& v) { write(key, QString::fromStdString(v)); }
    void write(const QString& key, const QString& v);
    void write(const QString& key, bool v);
    void write(const QString& key, int v);
    void write(const QString& key, unsigned int v);
    void write(const QString& key, long int v);
    void write(const QString& key, long unsigned int v);
    void write(const QString& key, float);
    void write(const QString& key, double);
    void write(const QString& key, const QDateTime& v);

    template <class T>
    XmlStream& operator << (const PairStruct<T>& p) { write(p.key, p.value); return *this; }

    // ----------------- data read ------------------

    /** @{ */
    /** Reads the contents for the given key.
        If the key is not found, the default value will be used and false is returned. */

    bool read(const QString& key, QString& v, const QString& def = "") const;
    bool read(const QString& key, bool& v, bool def = false) const;
    bool read(const QString& key, int& v, int def = 0) const;
    bool read(const QString& key, unsigned int& v, unsigned int def = 0) const;
    bool read(const QString& key, long int& v, long int def = 0) const;
    bool read(const QString& key, long unsigned int& v, long unsigned int def = 0) const;
    bool read(const QString& key, float& v, float def = 0) const;
    bool read(const QString& key, double& v, double def = 0) const;
    bool read(const QString& key, QDateTime& v, const QDateTime& def = QDateTime()) const;

    /** @} */

    QString readString(const QString& key, const QString& def = "") const { QString v; read(key, v, def); return v; }
    bool readBool(const QString& key, bool def = false) const { bool v; read(key, v, def); return v; }
    int readInt(const QString& key, int def = 0) const { int v; read(key, v, def); return v; }
    unsigned int readUInt(const QString& key, unsigned int def = 0) const { unsigned int v; read(key, v, def); return v; }
    long int readLInt(const QString& key, long int def = 0) const { long int v; read(key, v, def); return v; }
    long unsigned int readLUInt(const QString& key, long unsigned int def = 0) const { long unsigned int v; read(key, v, def); return v; }
    float readFloat(const QString& key, float def = 0) const { float v; read(key, v, def); return v; }
    double readDouble(const QString& key, double def = 0) const { double v; read(key, v, def); return v; }
    QDateTime readDateTime(const QString& key, const QDateTime& def = QDateTime()) { QDateTime v; read(key, v, def); return v; }

    // ------------- expect-read ---------------------

    /** @{ */
    /** Reads the contents for the given key.
        If the key is not found, an IoException is thrown. */

    void expect(const QString& key, QString& v) const;
    void expect(const QString& key, bool& v) const;
    void expect(const QString& key, int& v) const;
    void expect(const QString& key, unsigned int& v) const;
    void expect(const QString& key, long int& v) const;
    void expect(const QString& key, long unsigned int& v) const;
    void expect(const QString& key, float& v) const;
    void expect(const QString& key, double& v) const;
    void expect(const QString& key, QDateTime& v) const;

    /** @} */

    // TODO: type checking

    QString expectString(const QString& key) const { QString v; expect(key, v); return v; }
    bool expectBool(const QString& key) const { bool v; expect(key, v); return v; }
    int expectInt(const QString& key) const { int v; expect(key, v); return v; }
    unsigned int expectUInt(const QString& key) const { unsigned int v; expect(key, v); return v; }
    long int expectLInt(const QString& key) const { long int v; expect(key, v); return v; }
    long unsigned int expectLUInt(const QString& key) const { long unsigned int v; expect(key, v); return v; }
    float expectFloat(const QString& key) const { float v; expect(key, v); return v; }
    double expectDouble(const QString& key) const { double v; expect(key, v); return v; }
    QDateTime expectDateTime(const QString& key) const { QDateTime v; expect(key, v); return v; }

    // _______________ PRIVATE AREA _________________

private:
    QString cur_section_;
    std::vector<QString> section_stack_;

    QXmlStreamWriter * xmlw_;
    QXmlStreamReader * xmlr_;
    QString data_;
};

// -------------------- pair stream -------------------


} // namespace IO
} // namespace MO

#endif // MOSRC_IO_XMLSTREAM_H
