/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 3/3/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34_

#include "py_utils.h"
#include "python_timeline.h"
#include "python_vector.h"
#include "math/timeline1d.h"

namespace MO {
namespace PYTHON34 {


namespace {
extern "C" {

static const char* TimelineDocString()
{
    static const char* str = "The timeline";
    return str;
}


// ---------- helper -----------








// ------------- the python object --------------

struct TimelineStruct
{
    PyObject_HEAD
    MATH::Timeline1d* tl[4];
    int num;
};

static TimelineStruct* new_tl();
static TimelineStruct* copy_tl(TimelineStruct*);

static void tl_dealloc(TimelineStruct* self)
{
    for (int i=0; i<4; ++i)
    if (self->tl[i])
        self->tl[i]->releaseRef();
    self->ob_base.ob_type->tp_free((PyObject*)self);
}

static void tl_copy_from(TimelineStruct* self, const TimelineStruct* other)
{
    MATH::Timeline1d* tmp[4];
    for (int i=0; i<4; ++i)
        tmp[i] = self->tl[i];

    for (int i=0; i<4; ++i)
    {
        self->tl[i] = other->tl[i];
        if (self->tl[i])
            self->tl[i]->addRef();
    }

    for (int i=0; i<4; ++i)
        if (tmp[i])
            tmp[i]->releaseRef();
}

static int tl_init(TimelineStruct* self, PyObject* args_, PyObject*)
{
    if (isTimeline(args_))
    {
        auto other = reinterpret_cast<TimelineStruct*>(args_);
        tl_copy_from(self, other);
        return 0;
    }

    return 0;
}

static PyObject* tl_newfunc(PyTypeObject* type, PyObject* , PyObject* )
{
    auto self = reinterpret_cast<TimelineStruct*>(type->tp_alloc(type, 0));

    if (self != NULL)
    {
        for (int i=0; i<4; ++i)
            self->tl[i] = 0;
        self->tl[0] = new MATH::Timeline1d;
        self->num = 1;
    }

    return reinterpret_cast<PyObject*>(self);
}


// --------------------- getter ---------------------------

#define MO__ASSERT_TL(struct__) \
    if (struct__->num == 0 || struct__->num > 4) \
    { \
        PyErr_Set(PyExc_ReferenceError, \
                QString("attached timeline dimensionality %1 is bogus").arg(struct__->num)); \
        return NULL; \
    } \
    for (int i=0; i<struct__->num; ++i) \
    if (struct__->tl[i] == nullptr) \
    { \
        PyErr_Set(PyExc_ReferenceError, \
                QString("attached timeline #%1/%2 is NULL") \
                    .arg(i+1).arg(struct__->num)); \
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
        auto str = QString("Timeline(dim %1, points %2")
                          .arg(self->num, self->tl[0]->size());
        for (int i=1; i<self->num; ++i)
            str += QString("/%1").arg(self->tl[i]->size());
        return fromString(str + ")");
    }
    static PyObject* tl_repr(PyObject* self)
        { return tl_to_string(reinterpret_cast<TimelineStruct*>(self), nullptr); }

    MO_PY_DEF_DOC(tl_dimensions,
        "dimensions() -> i\n"
        "Returns the number of dimensions, e.g. the number of components per vector"
    )
    static PyObject* tl_dimensions(TimelineStruct* self, PyObject* )
    {
        return fromInt(self->num);
    }

    MO_PY_DEF_DOC(tl_size,
        "size() -> i\n"
        "Returns the number of points"
    )
    static PyObject* tl_size(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        unsigned int num = self->tl[0]->size();
        for (int i=1; i<self->num; ++i)
            num = std::max(num, self->tl[i]->size());
        return fromInt(num);
    }


    MO_PY_DEF_DOC(tl_value,
        "value(f) -> f | vec\n"
        "Returns the value at the given time"
    )
    static PyObject* tl_value(TimelineStruct* self, PyObject* arg)
    {
        MO__ASSERT_TL(self);
        double v;
        if (!toDouble(arg, &v))
            return NULL;
        if (self->num == 1)
            return fromDouble(self->tl[0]->get(v));
        if (self->num == 2)
            return buildVector(self->tl[0]->get(v), self->tl[1]->get(v));
        if (self->num == 3)
            return buildVector(self->tl[0]->get(v), self->tl[1]->get(v),
                               self->tl[2]->get(v));
        return buildVector(self->tl[0]->get(v), self->tl[1]->get(v),
                           self->tl[2]->get(v), self->tl[3]->get(v));
    }

    // ----------------- setter ---------------------

    MO_PY_DEF_DOC(tl_add,
        "add(vec2) -> Timeline\n"
        "Adds a point to the timeline.\n"
        "Returns self."
    )
    static PyObject* tl_add(TimelineStruct* self, PyObject* arg)
    {
        MO__ASSERT_TL(self->tl);
        double v[2];
        if (!get_vector(arg, 2, v))
            return NULL;
        self->tl->add(v[0], v[1]);
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

    MO__METHOD(value,               METH_O)

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

TimelineStruct* copy_tl(TimelineStruct* self)
{
    auto copy = PyObject_New(TimelineStruct, Timeline_Type());
    tl_copy_from(copy, self);
    return copy;
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

void* buildTimeline(MATH::Timeline1d* tl)
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
