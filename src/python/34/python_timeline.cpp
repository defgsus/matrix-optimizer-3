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
#include "math/TimelineNd.h"
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

bool py_get_time_and_vec(
        PyObject* args_, double* time, MATH::TimelineNd::ValueType* vec)
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

bool py_get_time_and_vec_and_pointtype(
        PyObject* args_, double* time, MATH::TimelineNd::ValueType* vec,
        int* ptype)
{
    if (vec->numDimensions() == 1)
    {
        if (PyArg_ParseTuple(args_, "ddi", time, &(*vec)[0], ptype))
            return true;
    }
    else if (vec->numDimensions() == 2)
    {
        if (PyArg_ParseTuple(args_, "dddi", time, &(*vec)[0], &(*vec)[1], ptype))
            return true;
    }
    else if (vec->numDimensions() == 3)
    {
        if (PyArg_ParseTuple(args_, "ddddi",
                             time, &(*vec)[0], &(*vec)[1], &(*vec)[2], ptype))
            return true;
    }
    else if (vec->numDimensions() == 4)
    {
        if (PyArg_ParseTuple(args_, "dddddi",
                time, &(*vec)[0], &(*vec)[1], &(*vec)[2], &(*vec)[3], ptype))
            return true;
    }
    PyErr_Clear();
    PyObject * second;
    if (!PyArg_ParseTuple(args_, "dOi", time, &second, ptype))
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
        self->tl->releaseRef("py timeline dealloc");
    self->ob_base.ob_type->tp_free((PyObject*)self);
}

static void tl_copy_from(TimelineStruct* self, const TimelineStruct* other)
{
    auto tmp = self->tl;

    self->tl = other->tl;
    if (self->tl)
        self->tl->addRef("py timeline copy-from-other");

    if (tmp)
        tmp->releaseRef("py timeline copy-from-other release-previous");
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

    MO_PY_DEF_DOC(tl_start,
        "start() -> float\n"
        "The time of the first cue in seconds"
    )
    static PyObject* tl_start(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        return fromDouble(self->tl->empty()
                          ? 0.0 : self->tl->getData().begin()->second.t);
    }

    MO_PY_DEF_DOC(tl_end,
        "end() -> float\n"
        "The time of the last cue in seconds"
    )
    static PyObject* tl_end(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        return fromDouble(self->tl->empty()
                          ? 0.0 : self->tl->getData().rbegin()->second.t);
    }

    MO_PY_DEF_DOC(tl_length,
        "length() -> float\n"
        "The time between the first and last cue in seconds"
    )
    static PyObject* tl_length(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        return fromDouble(self->tl->empty()
                          ? 0.0 : self->tl->getData().rbegin()->second.t
                                - self->tl->getData().begin()->second.t);
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
        "value(f) -> f | vec | list\n"
        "Returns the value at the given time.\n"
        "If dimension is 1, returns float.\n"
        "If dimension is 2-4, returns matrixoptimizer.Vec\n"
        "If dimension is >4, returns list of float"
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
            return buildVector(self->tl->get(time));
        return buildList(self->tl->get(time));
    }

    MO_PY_DEF_DOC(tl_derivative,
        "derivative(f, f) -> f | vec | list\n"
        "Returns the derivative at the given time.\n"
        "The optional second argument is the time range to observe, initialized to 0.01\n"
        "If dimension is 1, returns float.\n"
        "If dimension is 2-4, returns matrixoptimizer.Vec\n"
        "If dimension is >4, returns list of float"
    )
    static PyObject* tl_derivative(TimelineStruct* self, PyObject* arg)
    {
        MO__ASSERT_TL(self);
        double time, range = 0.01;
        if (!PyArg_ParseTuple(arg, "d|d", &time, &range))
            return NULL;
        range = .5001 * std::max(range, MATH::TimelineNd::timeQuantum());
        if (self->tl->numDimensions() == 1)
            return fromDouble(self->tl->getDerivative(time, range)[0]);
        if (self->tl->numDimensions() <= 4)
            return buildVector(self->tl->getDerivative(time, range));
        return buildList(self->tl->getDerivative(time, range));
    }

    MO_PY_DEF_DOC(tl_get_timeline,
        "get_timeline(i) -> Timeline | None\n"
        "Returns the i'th dimension as one-dimensional timeline.\n"
    )
    static PyObject* tl_get_timeline(TimelineStruct* self, PyObject* arg)
    {
        MO__ASSERT_TL(self);
        long idx = PyLong_AsLong(arg);
        if (idx < 0 || (size_t)idx >= self->tl->numDimensions())
        {
            PyErr_Set(PyExc_IndexError, QString("Dimension out of range %1/%2")
                      .arg(idx).arg(self->tl->numDimensions()));
            return NULL;
        }
        auto tl1 = self->tl->getTimelineNd(idx, 1);
        auto ret = reinterpret_cast<PyObject*>(buildTimeline(tl1));
        tl1->releaseRef("create finish");
        return ret;
    }


    MO_PY_DEF_DOC(tl_keys,
        "keys() -> set\n"
        "Returns a set containing the times of all timeline points\n"
    )
    static PyObject* tl_keys(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        auto set = PySet_New(NULL);
        if (!set)
        {
            PyErr_Set(PyExc_MemoryError, "Could not create set object");
            return NULL;
        }
        for (auto& i : self->tl->getData())
        {
            auto d = fromDouble(i.second.t);
            if (!d)
            {
                Py_DecRef(set);
                return NULL;
            }
            if (PySet_Add(set, d) < 0)
            {
                Py_DecRef(set);
                return NULL;
            }
        }
        return set;
    }

    // helper for MATH::TimelinePoint::Type getter
    static PyObject* tl_type_getter(TimelineStruct* , void* ptr)
    {
        return fromInt((int64_t)ptr);
    }

    // ----------------- setter ---------------------

    MO_PY_DEF_DOC(tl_set_dimensions,
        "set_dim(long) -> None\n"
        "Sets the number of dimensions which must be at least one.\n"
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

    /*
    MO_PY_DEF_DOC(tl_set_type,
        "set_type(long) -> None\n"
        "Sets the type of the timeline points that will be added afterwards.\n"
        "CONSTANT, LINEAR, SMOOTH, SYMMETRIC, SYMMETRIC_USER, HERMITE,\n"
        "SPLINE4, SPLINE6"
    )
    static PyObject* tl_set_type(TimelineStruct* self, PyObject* arg)
    {
        MO__ASSERT_TL(self);
        long typ;
        if (PyArg_ParseTuple(arg, "l", &typ))
        {
            //self->tl->setCurrentType(MATH::TimelinePoint::Type(typ));
        }
        Py_RETURN_NONE;
    }
    */


    MO_PY_DEF_DOC(tl_add,
        "add(float, vec) -> Timeline\n"
        "add(float, vec, int) -> Timeline\n"
        "Adds a value/vector at the given time.\n"
        "The vector size must fit the dimension of the timeline data.\n"
        "The third argument can be on of the Timeline point types,\n"
        "CONSTANT, LINEAR, SMOOTH, SYMMETRIC, SYMMETRIC_USER, HERMITE,\n"
        "SPLINE4, SPLINE6\n"
        "Returns self."
    )
    static PyObject* tl_add(TimelineStruct* self, PyObject* arg)
    {
        MO__ASSERT_TL(self);

        double time;
        MATH::TimelineNd::ValueType
                val(self->tl->numDimensions(), MATH::TimelineNd::ValueType::NoInit);
        int ptype = MATH::TimelinePoint::DEFAULT;
        if (!py_get_time_and_vec_and_pointtype(arg, &time, &val, &ptype))
        {
            MO_DEBUG("TODO Python Timeline: PTYPE not retrieved");
            PyErr_Clear();
            if (!py_get_time_and_vec(arg, &time, &val))
                return NULL;
        }
        self->tl->add(time, val, (MATH::TimelinePoint::Type)ptype);

        Py_INCREF(self);
        return reinterpret_cast<PyObject*>(self);
    }

    MO_PY_DEF_DOC(tl_update,
        "update() -> None\n"
        "Precalculates the derivatives of the timeline.\n"
        "This is needed for some interpolation modes."
    )
    static PyObject* tl_update(TimelineStruct* self, PyObject* )
    {
        MO__ASSERT_TL(self);
        self->tl->setAutoDerivative();
        Py_RETURN_NONE;
    }





#define MO__METHOD(name__, args__) \
    { #name__, (PyCFunction)tl_##name__, args__, tl_##name__##_doc },

static PyMethodDef Timeline_methods[] =
{
    MO__METHOD(to_string,           METH_NOARGS)
    MO__METHOD(dimensions,          METH_NOARGS)
    MO__METHOD(size,                METH_NOARGS)
    MO__METHOD(start,               METH_NOARGS)
    MO__METHOD(end,                 METH_NOARGS)
    MO__METHOD(length,              METH_NOARGS)

    MO__METHOD(copy,                METH_NOARGS)
    MO__METHOD(get_timeline,        METH_O)
    MO__METHOD(value,               METH_O)
    MO__METHOD(derivative,          METH_VARARGS)
    MO__METHOD(keys,                METH_NOARGS)

    MO__METHOD(set_dimensions,      METH_O)
    MO__METHOD(update,              METH_NOARGS)
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

#define MO__TYPE(name__, enum__, doc__) \
    { (char*)name__, (getter)tl_type_getter, (setter)NULL, \
      (char*)"interpolation type: " doc__, (void*)MATH::TimelinePoint::enum__ },

static PyGetSetDef Timeline_getseters[] = {
    MO__TYPE("CONSTANT"   , CONSTANT    , "no transition")
    MO__TYPE("LINEAR"     , LINEAR      , "two-point linear transition")
    MO__TYPE("SMOOTH"     , SMOOTH      , "two-point sigmoid transition")
    MO__TYPE("SYMMETRIC"  , SYMMETRIC   , "two-point smooth transition"
                                          "using calculated derivatives "
                                          " (differentiable)")
    MO__TYPE("SYMMETRIC_USER", SYMMETRIC_USER, "two-point smooth transition"
                                          "using user derivatives (differentiable)")
    MO__TYPE("HERMITE"    , HERMITE  ,    "two-point hermite-like transition "
                                          "using calculated derivatives")
    MO__TYPE("HERMITE_USER", HERMITE_USER, "two-point hermite-like transition "
                                          "using defined derivatives")
    MO__TYPE("SPLINE4"    , SPLINE4     , "four-point spline")
    MO__TYPE("SPLINE6"    , SPLINE6     , "six-point spline")

    { NULL, NULL, NULL, NULL, NULL }
};

#undef MO__TYPE

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
        0,//tl_repr,     /*tp_repr*/
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
        Timeline_getseters,    /* tp_getset */
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
    auto t = PyObject_New(TimelineStruct, Timeline_Type());
    t->tl = nullptr;
    return t;
}

TimelineStruct* copy_tl(TimelineStruct* self, bool newInstance)
{
    auto copy = PyObject_New(TimelineStruct, Timeline_Type());
    if (!newInstance)
        tl_copy_from(copy, self);
    else
    {
        if (copy->tl)
            copy->tl->releaseRef("py timeline copyfrom relprev");
        if (self->tl)
            copy->tl = new MATH::TimelineNd(*self->tl);
        else
            copy->tl = new MATH::TimelineNd(1);
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


void initTimeline(PyObject* mod)
{
    PyObject* module = reinterpret_cast<PyObject*>(mod);
    initObjectType(module, Timeline_Type(), "Timeline");
}

bool isTimeline(PyObject* obj)
{
    return PyObject_TypeCheck(obj, Timeline_Type());
}

bool expectTimeline(PyObject* obj)
{
    if (!obj)
    {
        PyErr_SetString(PyExc_TypeError, "expected Timeline, got NULL");
        return false;
    }
    if (!isTimeline(obj))
    {
        PyErr_Set(PyExc_TypeError, QString("expected Timeline, got %1")
                                    .arg(typeName(obj)));
        return false;
    }
    return true;
}

PyObject* buildTimeline(MO::MATH::TimelineNd* tl)
{
    auto obj = new_tl();
    if (obj->tl)
        obj->tl->releaseRef("py timeline buildfrom relprev");
    obj->tl = tl;
    if (tl)
        tl->addRef("py timeline buildfrom");
    return reinterpret_cast<PyObject*>(obj);
}

PyObject* buildList(const MO::MATH::ArithmeticArray<double>& a)
{
    auto list = PyList_New(a.numDimensions());
    for (size_t i=0; i<a.numDimensions(); ++i)
    {
        auto v = fromDouble(a[i]);
        if (!v)
        {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SET_ITEM(list, i, v);
    }
    return list;
}


MATH::TimelineNd* getTimeline(PyObject* o)
{
    if (!expectTimeline(o))
        return nullptr;
    return reinterpret_cast<TimelineStruct*>(o)->tl;
}

} // namespace PYTHON34
} // namespace MO


#endif // MO_ENABLE_PYTHON34
