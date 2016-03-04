/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/3/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include "py_utils.h"
#include "python_timeline.h"
#include "python_vector.h"
#include "math/timelinend.h"
#include "io/log.h"

namespace MO {
namespace PYTHON34 {


namespace {
extern "C" {

static const char* TimelineDocString()
{
    static const char* str = "N-Dimensional timeline";
    return str;
}


// ---------- helper -----------

struct TimelineStruct;

bool set_tl_dim(TimelineStruct*, size_t num);

bool py_get_time_and_vec(PyObject* args_, double* time, MATH::TimelineNd::ValueType* vec)
{
    if (vec->numDimensions() == 1)
    {
        if (PyArg_ParseTuple(args_, "dd", time, &(*vec)[0]))
            return true;
    }
    else if (vec->numDimensions() == 2)
    {
        if (PyArg_ParseTuple(args_, "ddd", time, &(*vec)[0], &(*vec)[1]))
            return true;
    }
    else if (vec->numDimensions() == 3)
    {
        if (PyArg_ParseTuple(args_, "dddd", time, &(*vec)[0], &(*vec)[1], &(*vec)[2]))
            return true;
    }
    else if (vec->numDimensions() == 4)
    {
        if (PyArg_ParseTuple(args_, "ddddd", time, &(*vec)[0], &(*vec)[1], &(*vec)[2], &(*vec)[3]))
            return true;
    }
    PyErr_Clear();
    PyObject * second;
    if (!PyArg_ParseTuple(args_, "dO", time, &second))
        return false;
    if (get_vector(second, vec->numDimensions(), &(*vec)[0]))
        return true;
    return false;
}





// ------------- the python object --------------

struct TimelineStruct
{
    PyObject_HEAD
    MATH::TimelineNd* tl;
};

static TimelineStruct* new_tl();
static TimelineStruct* copy_tl(TimelineStruct*, bool newInstance);

static void tl_dealloc(TimelineStruct* self)
{
    if (self->tl)
        self->tl->releaseRef();
    self->ob_base.ob_type->tp_free((PyObject*)self);
}

static void tl_copy_from(TimelineStruct* self, const TimelineStruct* other)
{
    auto tmp = self->tl;

    self->tl = other->tl;
    if (self->tl)
        self->tl->addRef();

    if (tmp)
        tmp->releaseRef();
}

static int tl_init(TimelineStruct* self, PyObject* args_, PyObject*)
{
    // copy_construct
    if (isTimeline(args_))
    {
        auto other = reinterpret_cast<TimelineStruct*>(args_);
        tl_copy_from(self, other);
        return 0;
    }
    long numDim;
    if (PyArg_ParseTuple(args_, "l", &numDim))
    {
        if (set_tl_dim(self, numDim))
            return 0;
    }

    return -1;
}

static PyObject* tl_newfunc(PyTypeObject* type, PyObject* , PyObject* )
{
    auto self = reinterpret_cast<TimelineStruct*>(type->tp_alloc(type, 0));

    if (self != NULL)
    {
        self->tl = new MATH::TimelineNd(1);
    }

    return reinterpret_cast<PyObject*>(self);
}


// --------------------- getter ---------------------------

#define MO__ASSERT_TL(struct__) \
    if (struct__->tl == nullptr) \
    { \
        PyErr_SetString(PyExc_ReferenceError, "attached timeline is NULL"); \
        return NULL; \
    } \
    if (struct__->tl->numDimensions() == 0) \
    { \
        PyErr_SetString(PyExc_ReferenceError, \
                "timeline dimensionality is NULL"); \
        return NULL; \
    }



#define MO_PY_DEF_DOC(name__, str__) \
    static const char* name__##_doc = str__;

    MO_PY_DEF_DOC(tl_to_string,
        "to_string() -> str\n"
        "Returns the string representation of the Timeline"
    )
    static PyObject* tl_to_string(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        return fromString(QString("Timeline(dim %1, points %2)")
                          .arg(self->tl->numDimensions())
                          .arg(self->tl->size()) );
    }
    static PyObject* tl_repr(PyObject* self)
        { return tl_to_string(reinterpret_cast<TimelineStruct*>(self), nullptr); }

    MO_PY_DEF_DOC(tl_dimensions,
        "dimensions() -> i\n"
        "Returns the number of dimensions, e.g. the number of components per vector"
    )
    static PyObject* tl_dimensions(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        return fromInt(self->tl->numDimensions());
    }

    MO_PY_DEF_DOC(tl_size,
        "size() -> i\n"
        "Returns the number of points"
    )
    static PyObject* tl_size(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        return fromInt(self->tl->size());
    }


    MO_PY_DEF_DOC(tl_copy,
        "copy() -> Timeline\n"
        "Returns a copy of the current timeline.\n"
        "The copy is a NEW instance."
    )
    static PyObject* tl_copy(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        return reinterpret_cast<PyObject*>(copy_tl(self, true));
    }


    MO_PY_DEF_DOC(tl_value,
        "value(f) -> f | vec\n"
        "Returns the value at the given time"
    )
    static PyObject* tl_value(TimelineStruct* self, PyObject* arg)
    {
        MO__ASSERT_TL(self);
        double time;
        if (!toDouble(arg, &time))
            return NULL;
        if (self->tl->numDimensions() == 1)
            return fromDouble(self->tl->get(time)[0]);
        if (self->tl->numDimensions() <= 4)
        {
            return reinterpret_cast<PyObject*>(buildVector(self->tl->get(time)));
        }
        // XXX
        Py_RETURN_NONE;
    }

    // ----------------- setter ---------------------

    MO_PY_DEF_DOC(tl_set_dimensions,
        "set_dim(long) -> None\n"
        "Sets the number of dimensions which must be in range [1,4]\n"
        "Each point in the timeline is affected. If the dimensionality grows,\n"
        "new data is initialized to zero."
    )
    static PyObject* tl_set_dimensions(TimelineStruct* self, PyObject* arg)
    {
        MO__ASSERT_TL(self);
        long numDim;
        if (PyArg_ParseTuple(arg, "l", &numDim))
        {
            if (!set_tl_dim(self, numDim))
                return NULL;
        }
        Py_RETURN_NONE;
    }


    MO_PY_DEF_DOC(tl_add,
        "add(vec2) -> Timeline\n"
        "Adds a point to the timeline.\n"
        "Returns self."
    )
    static PyObject* tl_add(TimelineStruct* self, PyObject* arg)
    {
        MO__ASSERT_TL(self);

        double time;
        MATH::TimelineNd::ValueType
                val(self->tl->numDimensions(), MATH::TimelineNd::ValueType::NoInit);
        if (!py_get_time_and_vec(arg, &time, &val))
            return NULL;
        MO_PRINT("ADD " << time << ", " << val);
        self->tl->add(time, val);

        Py_INCREF(self);
        return reinterpret_cast<PyObject*>(self);
    }







#define MO__METHOD(name__, args__) \
    { #name__, (PyCFunction)tl_##name__, args__, tl_##name__##_doc },

static PyMethodDef Timeline_methods[] =
{
    MO__METHOD(to_string,           METH_NOARGS)
    MO__METHOD(dimensions,          METH_NOARGS)
    MO__METHOD(size,                METH_NOARGS)

    MO__METHOD(copy,                METH_NOARGS)
    MO__METHOD(value,               METH_O)

    MO__METHOD(set_dimensions,      METH_O)
    MO__METHOD(add,                 METH_VARARGS)

    { NULL, NULL, 0, NULL }
};
#undef MO__METHOD

#if 0
PySequenceMethods Timeline_SeqMethods = {
    (lenfunc)tl_sq_length, /* lenfunc sq_length */
    NULL, /* binaryfunc sq_concat */
    NULL, /* ssizeargfunc sq_repeat */
    (ssizeargfunc)tl_sq_item, /* ssizeargfunc sq_item */
    NULL, /* void *was_sq_slice */
    (ssizeobjargproc)tl_sq_ass_item, /* ssizeobjargproc sq_ass_item */
    NULL, /* void *was_sq_ass_slice */
    NULL, /* objobjproc sq_contains */
    NULL, /* binaryfunc sq_inplace_concat */
    NULL  /* ssizeargfunc sq_inplace_repeat */
};
#endif

static PyTypeObject* Timeline_Type()
{
    static PyTypeObject type =
    {
        PyVarObject_HEAD_INIT(NULL, 0)
        "matrixoptimizer.Timeline",  /*tp_name*/
        sizeof(TimelineStruct),  /*tp_basicsize*/
        0,                         /*tp_itemsize*/
        (destructor)tl_dealloc,/*tp_dealloc*/
        0,                         /*tp_print*/
        0,                         /*tp_getattr*/
        0,                         /*tp_setattr*/
        0,                         /*tp_reserved*/
        0,//vec_repr,     /*tp_repr*/
        0,//&Timeline_NumMethods,         /*tp_as_number*/
        0,//&Timeline_SeqMethods,        /*tp_as_sequence*/
        0,                         /*tp_as_mapping*/
        0,                         /*tp_hash */
        0,                         /*tp_call*/
        tl_repr,     /*tp_str*/
        PyObject_GenericGetAttr,   /*tp_getattro*/
        PyObject_GenericSetAttr,   /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
        TimelineDocString(),   /* tp_doc */
        0,		               /* tp_traverse */
        0,		               /* tp_clear */
        0,		               /* tp_richcompare */
        0,		               /* tp_weaklistoffset */
        0,//TimelineFuncs::get_iter,	/* tp_iter */
        0,//TimelineFuncs::next_iter, /* tp_iternext */
        Timeline_methods,      /* tp_methods */
        0,//Timeline_members,      /* tp_members */
        0,//Timeline_getseters,    /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        (initproc)tl_init,      /* tp_init */
        0,                         /* tp_alloc */
        (newfunc)tl_newfunc,       /* tp_new */
        0, /*tp_free*/
        0, /*tp_is_gc*/
        0, /*tp_bases*/
        0, /*tp_mro*/
        0, /*tp_cache*/
        0, /*tp_subclasses*/
        0, /*tp_weaklist*/
        0, /*tp_del*/
        0, /*tp_version_tag*/
        0, /*tp_finalize*/
    #ifdef COUNT_ALLOCS
        0, /*tp_allocs*/
        0, /*tp_frees*/
        0, /*tp_maxalloc*/
        0, /*tp_prev*/
        0, /*tp_next*/
    #endif
    };
    return &type;
}


TimelineStruct* new_tl()
{
    return PyObject_New(TimelineStruct, Timeline_Type());
}

TimelineStruct* copy_tl(TimelineStruct* self, bool newInstance)
{
    auto copy = PyObject_New(TimelineStruct, Timeline_Type());
    if (!newInstance)
        tl_copy_from(copy, self);
    else
    {
        auto tmp = copy->tl;
        if (copy->tl)
            copy->tl->releaseRef();
        if (self->tl)
            copy->tl = new MATH::TimelineNd(*self->tl);
        else
            copy->tl = new MATH::TimelineNd(1);
        if (tmp)
            tmp->releaseRef();
    }
    return copy;
}

bool set_tl_dim(TimelineStruct* self, size_t num)
{
    if (num<1 || num>4)
    {
        PyErr_Set(PyExc_TypeError,
                  QString("number of dimensions out of range 1<=%1<=4")
                  .arg(num));
        return false;
    }
    self->tl->setDimensions(num);
    return true;
}

} // extern "C"
} // namespace


void initTimeline(void* mod)
{
    PyObject* module = reinterpret_cast<PyObject*>(mod);
    initObjectType(module, Timeline_Type(), "Timeline");
}

bool isTimeline(void* vobj)
{
    return PyObject_TypeCheck(reinterpret_cast<PyObject*>(vobj), Timeline_Type());
}

void* buildTimeline(MO::MATH::TimelineNd* tl)
{
    auto obj = new_tl();
    auto tmp = obj->tl;
    if (obj->tl)
        obj->tl->releaseRef();
    obj->tl = tl;
    if (tl)
        tl->addRef();
    if (tmp)
        tmp->releaseRef();
    return obj;
}


} // namespace PYTHON34
} // namespace MO


#endif // MO_ENABLE_PYTHON34
