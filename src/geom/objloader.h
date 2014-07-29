/** @file objloader.h

    @brief Wavefront .obj loader

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/29/2014</p>
*/

#ifndef MOSRC_GEOM_OBJLOADER_H
#define MOSRC_GEOM_OBJLOADER_H

#include <vector>

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

    struct Vertex
    {
        UInt v, t, n;
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
        @throws IoExpection on parsing errors */
    void loadFromMemory(const QByteArray& a);

    // -------------- getter ------------------

    /** Returns wheter the ObjLoader contains valid vertex data */
    bool isEmpty() const;

    /** Load the data into the Geometry container. */
    void getGeometry(Geometry *) const;


private:

    bool readFaceVertex_(const QString&, int& x, Vertex&, bool expect) const;

    QString filename_;

    std::vector<Float>
        vertex_, texCoord_, normal_;

    std::vector<Vertex> triangle_;

};

} // namespace GEOM
} // namespace MO

#endif // MOSRC_GEOM_OBJLOADER_H
