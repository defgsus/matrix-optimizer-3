/***************************************************************************

Copyright (C) 2016  stefan.berke @ modular-audio-graphics.com

This source is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 3.0 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this software; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

****************************************************************************/

#ifdef MO_ENABLE_NIFTI
#include <nifti/nifti1_io.h>
#endif

#include "NiftiDO.h"
#include "object/param/Parameters.h"
#include "object/param/ParameterFilename.h"
#include "object/param/ParameterInt.h"
#include "object/param/ParameterSelect.h"
#include "math/interpol.h"
#include "io/DataStream.h"
#include "io/log.h"

namespace MO {

MO_REGISTER_OBJECT(NiftiDO)

struct NiftiDO::Private
{
    Private(NiftiDO* p)
        : p         (p)
        , doReload  (true)
    { }


    bool loadNifti(const QString& filename);

    NiftiDO* p;
    bool doReload;
    ParameterFilename* p_filename;
    FloatMatrix matrix;
};

NiftiDO::NiftiDO()
    : Object    ()
    , p_        (new Private(this))
{
    setName("Nifti");
    setNumberOutputs(ST_FLOAT_MATRIX, 1);
#ifndef MO_ENABLE_NIFTI
    setErrorMessage(tr("Nifti reading not enabled in this version"));
#endif
}

NiftiDO::~NiftiDO() { delete p_; }

void NiftiDO::serialize(IO::DataStream & io) const
{
    Object::serialize(io);
    io.writeHeader("nifti", 1);
}

void NiftiDO::deserialize(IO::DataStream & io)
{
    Object::deserialize(io);
    io.readHeader("nifti", 1);
}

void NiftiDO::createParameters()
{
    Object::createParameters();

    params()->beginParameterGroup("modval", tr("value"));
    initParameterGroupExpanded("modval");

        p_->p_filename = params()->createFilenameParameter(
            "filename", tr("filename"),
            tr("The filename of the nifti file"),
            IO::FT_NIFTI);

    params()->endParameterGroup();
}

void NiftiDO::onParameterChanged(Parameter* p)
{
    if (p == p_->p_filename)
        p_->doReload = true;
}

FloatMatrix NiftiDO::valueFloatMatrix(uint, const RenderTime& ) const
{
    if (p_->doReload)
    {
        if (!p_->loadNifti(p_->p_filename->value()))
            p_->matrix.setDimensions({1,1,1});
        //p_->matrix.normalize();
        p_->doReload = false;
    }
    return p_->matrix;
}

bool NiftiDO::Private::loadNifti(const QString& filename)
{
#ifdef MO_ENABLE_NIFTI

    auto img = nifti_image_read(filename.toStdString().c_str(), true);
    if (!img)
    {
        p->setErrorMessage(tr("Failed to load nifti file"));
        return false;
    }

    p->clearError();
    MO_PRINT(img->nx << "x" << img->ny << "x" << img->nz
             << " bytes=" << img->nbyper
             << ", dt=" << nifti_datatype_string(img->datatype));

    matrix.setDimensions({ (size_t)img->nz, (size_t)img->ny, (size_t)img->nx });

    MO_ASSERT(img->nvox == matrix.size(), "");
    switch (img->datatype)
    {
        default:
            p->setErrorMessage(tr("Datatype %1 not supported")
                .arg(nifti_datatype_to_string(img->datatype)) );
        break;
        case NIFTI_TYPE_INT8:
            for (size_t i=0; i<img->nvox; ++i)
                *matrix.data(i) = static_cast<int8_t*>(img->data)[i];
        break;
        case NIFTI_TYPE_INT16:
            for (size_t i=0; i<img->nvox; ++i)
                *matrix.data(i) = static_cast<int16_t*>(img->data)[i];
        break;
        case NIFTI_TYPE_INT32:
            for (size_t i=0; i<img->nvox; ++i)
                *matrix.data(i) = static_cast<int32_t*>(img->data)[i];
        break;

        case NIFTI_TYPE_UINT8:
            for (size_t i=0; i<img->nvox; ++i)
                *matrix.data(i) = static_cast<uint8_t*>(img->data)[i];
        break;
        case NIFTI_TYPE_UINT16:
            for (size_t i=0; i<img->nvox; ++i)
                *matrix.data(i) = static_cast<uint16_t*>(img->data)[i];
        break;
        case NIFTI_TYPE_UINT32:
            for (size_t i=0; i<img->nvox; ++i)
                *matrix.data(i) = static_cast<uint32_t*>(img->data)[i];
        break;

        case NIFTI_TYPE_FLOAT32:
            for (size_t i=0; i<img->nvox; ++i)
                *matrix.data(i) = static_cast<float*>(img->data)[i];
        break;
        case NIFTI_TYPE_FLOAT64:
            for (size_t i=0; i<img->nvox; ++i)
                *matrix.data(i) = static_cast<double*>(img->data)[i];
        break;
        case NIFTI_TYPE_FLOAT128:
            for (size_t i=0; i<img->nvox; ++i)
                *matrix.data(i) = static_cast<long double*>(img->data)[i];
        break;
    }

    nifti_image_free(img);

    return true;
#else

    return false;
#endif
}


} // namespace MO
