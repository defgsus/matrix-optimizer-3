/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include <python3.4/Python.h>
#include <python3.4/structmember.h>
#undef T_NONE
#undef T_OBJECT

#include "object/object.h"
#include "python_object.h"
#include "io/error.h"

namespace MO {
namespace PYTHON34 {

namespace {
extern "C" {

    PyObject* createBaseObjectInstance(Object*);

    struct BaseObjectStruct
    {
        PyObject_HEAD
        Object* object;
        int iterIdx;

        static constexpr const char* docString =
                "The object base";

        static void dealloc(BaseObjectStruct* self)
        {
            if (self->object)
                self->object->releaseRef();
            self->ob_base.ob_type->tp_free((PyObject*)self);
        }

        static int init(BaseObjectStruct* self, PyObject* args, PyObject*)
        {
            auto own = self->object;

            PyObject* obj = 0;
            PyArg_ParseTuple(args, "|O", &obj);
            if (obj && 0==strcmp(obj->ob_type->tp_name, "matrixoptimizer.Object"))
            {
                auto other = reinterpret_cast<BaseObjectStruct*>(obj);
                self->object = other->object;
                if (self->object)
                    self->object->addRef();
                self->iterIdx = other->iterIdx;
            }
            else
            {
                self->object = 0;
                self->iterIdx = 0;
            }

            if (own)
                own->releaseRef();

            return 0;
        }

        static PyObject* newfunc(PyTypeObject* type, PyObject* args, PyObject* kwds)
        {
            BaseObjectStruct* self =
                    reinterpret_cast<BaseObjectStruct*>(type->tp_alloc(type, 0));

            if (self != NULL)
            {
                self->object = nullptr;
                self->iterIdx = 0;
                init(self, args, kwds);
            }

            return reinterpret_cast<PyObject*>(self);
        }

    };



    struct BaseObjectFuncs
    {
        #define MO__GETOBJ0(name__) \
            if (self == nullptr) \
            { \
                PyErr_SetString(PyExc_RuntimeError, "self is NULL"); \
                return NULL; \
            } \
            auto name__ = reinterpret_cast<BaseObjectStruct*>(self);

        #define MO__GETOBJ(name__) \
            MO__GETOBJ0(name__) \
            if (name__->object == nullptr) \
                { Py_RETURN_NONE; }

        static PyObject* to_string(PyObject* self, PyObject* )
        {
            MO__GETOBJ0(pobj);
            if (!pobj->object)
                return Py_BuildValue("s", "*empty*");
            auto str = QString("%1{%2} (refcnt:%3)")
                    .arg(pobj->object->namePath())
                    .arg(pobj->object->className())
                    .arg(self->ob_refcnt);
            return Py_BuildValue("s", str.toLatin1().constData());
        }
        static PyObject* repr(PyObject* self) { return to_string(self, nullptr); }

        static PyObject* name(PyObject* self, PyObject* )
        {
            MO__GETOBJ(pobj);
            return Py_BuildValue("s",
                        pobj->object->name().toLatin1().constData());
        }

        static PyObject* id(PyObject* self, PyObject* )
        {
            MO__GETOBJ(pobj);
            return Py_BuildValue("s",
                        pobj->object->idName().toLatin1().constData());
        }

        static PyObject* name_path(PyObject* self, PyObject* )
        {
            MO__GETOBJ(pobj);
            return Py_BuildValue("s",
                        pobj->object->namePath().toLatin1().constData());
        }

        static PyObject* id_path(PyObject* self, PyObject* )
        {
            MO__GETOBJ(pobj);
            return Py_BuildValue("s",
                        pobj->object->idNamePath().toLatin1().constData());
        }

        static PyObject* class_name(PyObject* self, PyObject* )
        {
            MO__GETOBJ(pobj);
            return Py_BuildValue("s",
                        pobj->object->className().toLatin1().constData());
        }

        static PyObject* get_iter(PyObject* self)
        {
            MO__GETOBJ(pobj);
            pobj->iterIdx = 0;
            Py_INCREF(self);
            return self;
        }

        static PyObject* next_iter(PyObject* self)
        {
            MO__GETOBJ(pobj);
            if (pobj->object == 0
                    || pobj->iterIdx >= pobj->object->childObjects().size())
            {
                PyErr_SetNone(PyExc_StopIteration);
                return NULL;
            }
            return createBaseObjectInstance(
                            pobj->object->childObjects()[pobj->iterIdx++]);
        }

        #undef MO__GETOBJ
        #undef MO__GETOBJ0
    };

    PyMemberDef BaseObject_members[] =
    {
        { NULL, 0, 0, 0, NULL }
    };

    PyMethodDef BaseObject_methods[] =
    {
        { "to_string",
          (PyCFunction)BaseObjectFuncs::to_string,
          METH_NOARGS,
          "to_string() -> string\n"
          "Returns an informative string"
        },

        { "name",
          (PyCFunction)BaseObjectFuncs::name,
          METH_NOARGS,
          "name() -> string\n"
          "Returns the user-defined name of the object"
        },

        { "name_path",
          (PyCFunction)BaseObjectFuncs::name_path,
          METH_NOARGS,
          "name_path() -> string\n"
          "Returns the full path using the user-defined names objects"
        },

        { "id",
          (PyCFunction)BaseObjectFuncs::id,
          METH_NOARGS,
          "id() -> string\n"
          "Returns the unique id of the object"
        },

        { "id_path",
          (PyCFunction)BaseObjectFuncs::id_path,
          METH_NOARGS,
          "id_path() -> string\n"
          "Returns the full path using the IDs of the objects"
        },

        { "class_name",
          (PyCFunction)BaseObjectFuncs::class_name,
          METH_NOARGS,
          "class_name() -> string\n"
          "Returns the name of the object class"
        },

        { NULL, NULL, 0, NULL }
    };




    static PyTypeObject BaseObject_type =
    {
        PyVarObject_HEAD_INIT(NULL, 0)
        "matrixoptimizer.Object",  /*tp_name*/
        sizeof(BaseObjectStruct),  /*tp_basicsize*/
        0,                         /*tp_itemsize*/
        (destructor)BaseObjectStruct::dealloc,/*tp_dealloc*/
        0,                         /*tp_print*/
        0,                         /*tp_getattr*/
        0,                         /*tp_setattr*/
        0,                         /*tp_reserved*/
        0,                         /*tp_repr*/
        0,                         /*tp_as_number*/
        0,                         /*tp_as_sequence*/
        0,                         /*tp_as_mapping*/
        0,                         /*tp_hash */
        0,                         /*tp_call*/
        BaseObjectFuncs::repr,    /*tp_str*/
        0,                         /*tp_getattro*/
        0,                         /*tp_setattro*/
        0,                         /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
        BaseObjectStruct::docString,   /* tp_doc */
        0,		               /* tp_traverse */
        0,		               /* tp_clear */
        0,		               /* tp_richcompare */
        0,		               /* tp_weaklistoffset */
        BaseObjectFuncs::get_iter,	/* tp_iter */
        BaseObjectFuncs::next_iter, /* tp_iternext */
        BaseObject_methods,      /* tp_methods */
        BaseObject_members,      /* tp_members */
        0,                         /* tp_getset */
        0,                         /* tp_base */
        0,                         /* tp_dict */
        0,                         /* tp_descr_get */
        0,                         /* tp_descr_set */
        0,                         /* tp_dictoffset */
        (initproc)BaseObjectStruct::init,      /* tp_init */
        0,                         /* tp_alloc */
        (newfunc)BaseObjectStruct::newfunc,       /* tp_new */
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

    PyObject* createBaseObjectInstance(Object*o)
    {
        auto pobj = PyObject_New(BaseObjectStruct, &BaseObject_type);
        if (!pobj)
            return nullptr;
        pobj->object = o;
        if (o)
            o->addRef();
        return reinterpret_cast<PyObject*>(pobj);
    }

} // extern "C"
} // namespace

void initObject(void* mod)
{
    PyObject* module = reinterpret_cast<PyObject*>(mod);

    if (0 != PyType_Ready(&BaseObject_type))
        MO_ERROR("Failed to readify Object wrapper with Python 3.4");

    //if (0 != PyType_Ready(&BaseObjectIter_type))
    //    MO_ERROR("Failed to readify Object iterator with Python 3.4");

    PyObject* object = reinterpret_cast<PyObject*>(&BaseObject_type);
    Py_INCREF(object);
    if (0 != PyModule_AddObject(module, "Object", object))
    {
        Py_DECREF(object);
        MO_ERROR("Failed to add Object wrapper to Python 3.4");
    }
}

void* createObjectWrapper(Object* o)
{
    if (auto pobj = createBaseObjectInstance(o))
        return pobj;
    else
        MO_ERROR("Could not create Object wrapper for Python 3.7");
}

} // namespace PYTHON34
} // namespace MO

#endif // MO_ENABLE_PYTHON34
