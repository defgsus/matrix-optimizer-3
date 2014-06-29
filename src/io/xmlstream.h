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

class QXmlStreamWriter;
class QXmlStreamReader;

namespace MO {
namespace IO {

template <class T>
struct PairStruct
{
    PairStruct(const QString& key, const T value)
        :   key(key), value(value) { }

    QString key;
    T value;
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

    // --------------- streaming --------------------

    void startWriting();
    void stopWriting();
    bool isWriteable();

    void startReading();
    void stopReading();
    bool isReadable();

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

    /** @} */

    QString readString(const QString& key, const QString& def = "") const { QString v; read(key, v, def); return v; }
    bool readBool(const QString& key, bool def = false) const { bool v; read(key, v, def); return v; }
    int readInt(const QString& key, int def = 0) const { int v; read(key, v, def); return v; }
    unsigned int readUInt(const QString& key, unsigned int def = 0) const { unsigned int v; read(key, v, def); return v; }
    long int readLInt(const QString& key, long int def = 0) const { long int v; read(key, v, def); return v; }
    long unsigned int readLUInt(const QString& key, long unsigned int def = 0) const { long unsigned int v; read(key, v, def); return v; }
    float readFloat(const QString& key, float def = 0) const { float v; read(key, v, def); return v; }
    double readDouble(const QString& key, double def = 0) const { double v; read(key, v, def); return v; }

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
