/** @file geometryfactory.h

    @brief Creator of Geometry

    <p>(c) 2014, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/26/2014</p>
*/

#ifndef MOSRC_GL_GEOMETRYFACTORY_H
#define MOSRC_GL_GEOMETRYFACTORY_H



namespace MO {
namespace GL {

class Geometry;

class GeometryFactory
{
public:

    static void createCube(Geometry *, float side_length);
    static void createBox(Geometry *,
            float side_length_x, float side_length_y, float side_length_z);

    static void createGrid(Geometry *, int size, bool with_coordinate_system);
};



} // namespace GL
} // namespace MO


#endif // MOSRC_GL_GEOMETRYFACTORY_H
