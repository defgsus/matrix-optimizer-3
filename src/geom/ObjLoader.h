/** @file objloader.h

    @brief Wavefront .obj loader

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/29/2014</p>
*/

#ifndef MOSRC_GEOM_OBJLOADER_H
#define MOSRC_GEOM_OBJLOADER_H

#include <vector>

#include <QMap>
#include <QMutex>

#include <QString>
#include <QByteArray>

namespace MO {
namespace GEOM {

class Geometry;

class ObjLoader
{
public:

    typedef unsigned int UInt;
    typedef float Float;

    struct Material
    {
        QString name;
        Float a_r, a_g, a_b,
              d_r, d_g, d_b,
              s_r, s_g, s_b,
              alpha;
    };

    struct Vertex
    {
        UInt v, t, n;
        Material * mat;
    };

    static const int vertexComponents = 4;
    static const int normalComponents = 3;
    static const int texCoordComponents = 3;

    ObjLoader();

    /** Clears all internal data */
    void clear();

    /** Loads the file into internal data.
        @throws IoException on file or parsing errors. */
    void loadFile(const QString& filename);

    /** Loads the text content in @p a into internal data.
        @throws IoExpection on parsing errors.
        @note library files will not be loaded if loadFile() was not used. */
    void loadFromMemory(const QByteArray& a);

    // -------------- getter ------------------

    /** Returns whether the ObjLoader contains valid vertex data */
    bool isEmpty() const;

    /** Returns whether the ObjLoader is currently parsing an .obj */
    bool isLoading() const { return isLoading_; }

    /** Returns the textual log of the loading process */
    const QString& getLog() const { return log_; }

    /** Loads the data into the Geometry container.
        Previous contents will be kept. */
    void getGeometry(Geometry *) const;

    /** Returns the progress during loading [0,100] */
    int progress() const { return progress_; }

    // ----- buffer singleton access ----------

    /** Loads the data into the Geometry container.
        Previous contents will be kept.
        An ObjLoader class will be created for the file and kept.
        Throws IO::Exception on any errors. */
    static void getGeometry(const QString& filename, Geometry *);

private:

    bool readFaceVertex_(const QString&, int& x, Vertex&, bool expect) const;

    bool loadMaterialLib_(const QString& filename);

    void initMaterial_(Material& ) const;

    QString filename_,
            log_;

    volatile int progress_;
    volatile bool isLoading_;

    std::vector<Float>
        vertex_, texCoord_, normal_;

    std::vector<Vertex> triangle_, line_;

    QMap<QString, Material> material_;
    QMap<UInt, QString> materialUse_;

    static std::map<QString, ObjLoader*> instances_;
    static QMutex instanceMutex_;
};

} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_OBJLOADER_H
