/** @file io.h

    @brief Serializer and deserializer

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>

    <p>created 6/25/2014</p>
*/

#ifndef MOSCR_IO_IO_H
#define MOSCR_IO_IO_H

#include <vector>

#include <QString>

class QXmlStreamWriter;
class QXmlStreamReader;

namespace MO {

/** @brief serializer and deserializer
    <p>The backend is QXmlStream, so this is actually
    a convenience wrapper around it.</p>
*/
class Io
{
public:

    Io();
    ~Io();

    // -------------- file io -----------------------

    bool save(const QString& filename);
    bool load(const QString& filename);

    // --------------- streaming --------------------

    bool startWriting();
    bool stopWriting();
    bool writeable();

    bool startReading();
    bool stopReading();
    bool readable();

    // ------------------ sections ------------------

    /** Create a new (sub-)section. WRITE */
    bool newSection(const QString& name);

    /** Ends the current sub-section and goes to containing section. WRITE */
    bool endSection();

    /** Returns if the current section matches @p name. READ/WRITE */
    bool isSection(const QString& name) const;

    /** Returns name of the current section. READ/WRITE */
    const QString& section() const;

    /** open next sub-section. READ */
    bool nextSubSection();

    /** leave current (sub-)section. READ */
    bool leaveSection();

    // ----------------- data write -----------------

    bool write(const QString& key, const QString& v);
    bool write(const QString& key, bool v);
    bool write(const QString& key, int v);
    bool write(const QString& key, unsigned int v);
    bool write(const QString& key, long int v);
    bool write(const QString& key, long unsigned int v);
    bool write(const QString& key, float);
    bool write(const QString& key, double);

    // ----------------- data read ------------------

    bool read(const QString& key, QString& v, const QString& def = "") const;
    bool read(const QString& key, bool& v, bool def = false) const;
    bool read(const QString& key, int& v, int def = 0) const;
    bool read(const QString& key, unsigned int& v, unsigned int def = 0) const;
    bool read(const QString& key, long int& v, long int def = 0) const;
    bool read(const QString& key, long unsigned int& v, long unsigned int def = 0) const;
    bool read(const QString& key, float& v, float def = 0) const;
    bool read(const QString& key, double& v, double def = 0) const;

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

} // namespace MO

#endif // MOSRC_IO_IO_H
