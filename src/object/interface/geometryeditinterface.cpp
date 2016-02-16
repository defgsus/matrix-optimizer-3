/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/10/2016</p>
*/

#include "geometryeditinterface.h"
#include "gui/geometrydialog.h"

namespace MO {

GeometryEditInterface::~GeometryEditInterface()
{
    if (p_gei_diag_)
        p_gei_diag_->close();
}

void GeometryEditInterface::setAttachedGeometryDialog(GUI::GeometryDialog* d)
{
    p_gei_diag_ = d;
    if (d)
        d->setAttribute(Qt::WA_DeleteOnClose);
}

} // namespace MO
