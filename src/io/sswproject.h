/** @file

    @brief

    <p>(c) 2015, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 7/12/2015</p>
*/

#ifndef MOSRC_IO_SSWPROJECT_H
#define MOSRC_IO_SSWPROJECT_H

#include <QString>

namespace MO {

class JsonTreeModel;

/** SpatialSound Wave project loader.

    [SpatialSound Wave is copyright Fraunhofer Insitute]
*/
class SswProject
{
public:
    SswProject();
    ~SswProject();


    /** Loads a uifm file.
        @throws IoException on any error */
    void load(const QString& name);

    /** Creates a tree-model with the json data */
    JsonTreeModel * createModel() const;

private:
    struct Private;
    Private * p_;
};

} // namespace MO

#endif // MOSRC_IO_SSWPROJECT_H
