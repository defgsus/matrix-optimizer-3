/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include "py_utils.h"

#include "object/object.h"
#include "object/util/objecteditor.h"
#include "object/util/objectfactory.h"
#include "python_object.h"
#include "io/error.h"

namespace MO {
namespace PYTHON34 {

namespace {


    bool sortObjectList_ClassName(const Object * o1, const Object * o2)
    {
        return o1->className() < o2->className();
    }
    bool sortObjectList_Priority(const Object * o1, const Object * o2)
    {
        return ObjectFactory::objectPriority(o1) > ObjectFactory::objectPriority(o2);
    }

extern "C" {

    struct BaseObjectStruct;

    PyObject* createBaseObjectInstance(Object*);
    BaseObjectStruct* BaseObject_cast(PyObject*);

    const char* baseObjectDocString()
    {
        static QByteArray docUtf8;
        static const char* docChar = 0;
        if (docChar == 0)
        {
            QString str = "The matrixoptimizer object\n\n";

            str += "All classes:\n";
            auto list = ObjectFactory::objects(Object::TG_ALL);
            qSort(list.begin(), list.end(), sortObjectList_ClassName);
            qStableSort(list.begin(), list.end(), sortObjectList_Priority);
            int curprio = -1000;
            for (auto o : list)
            {
                if (curprio != ObjectFactory::objectPriority(o))
                {
                    curprio = ObjectFactory::objectPriority(o);
                    str += ObjectFactory::objectPriorityName(curprio) + ":\n";
                }
                str += "  " + o->className() + "\n";
            }

            docUtf8 = str.toUtf8();
            docChar = docUtf8.constData();
        }
        return docChar;
    }

    struct BaseObjectStruct
    {
        PyObject_HEAD
        Object* object;
        ObjectEditor* editor;
        int iterIdx;

        static void dealloc(BaseObjectStruct* self)
        {
            if (self->object)
                self->object->releaseRef();
            self->ob_base.ob_type->tp_free((PyObject*)self);
        }

        static int init(BaseObjectStruct* self, PyObject* args_, PyObject*)
        {
            auto own = self->object;
            self->object = nullptr;
            self->editor = nullptr;
            self->iterIdx = 0;
            int ret = 0;

            PyObject* arg = 0;
            PyArg_ParseTuple(args_, "|O", &arg);
            if (arg)
            {
                // copy constructor
                if (auto other = BaseObject_cast(arg))
                {
                    self->object = other->object;
                    if (self->object)
                        self->object->addRef();
                }
                else if (PyUnicode_Check(arg))
                {
                    const char* utf8 = PyUnicode_AsUTF8(arg);
                    auto str = QString::fromUtf8(utf8);
                    auto newo = ObjectFactory::createObject(str);
                    if (!newo)
                    {
                        PyErr_SetString(PyExc_TypeError,
                                        QString("object class '%1' unknown")
                                        .arg(str).toUtf8().constData());
                        ret = -1;
                    }
                    else
                    {
                        self->object = newo;
                    }
                }
            }
            else
            {
                self->object = 0;
            }

            if (own)
                own->releaseRef();
            if (self->object)
                self->editor = self->object->editor();

            return ret;
        }

        static PyObject* newfunc(PyTypeObject* type, PyObject* , PyObject* )
        {
            BaseObjectStruct* self =
                    reinterpret_cast<BaseObjectStruct*>(type->tp_alloc(type, 0));

            if (self != NULL)
            {
                self->object = nullptr;
                self->editor = 0;
                self->iterIdx = 0;
                //init(self, args, kwds);
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

        // ----------------- getter -----------------------

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

        static PyObject* num_children(PyObject* self, PyObject* )
        {
            MO__GETOBJ(o);
            return Py_BuildValue("n", o->object->numChildren());
        }

        static PyObject* children(PyObject* self, PyObject* args_)
        {
            MO__GETOBJ(o);
            long idx = -1;
            if (!PyArg_ParseTuple(args_, "|l", &idx))
                return NULL;
            if (idx >= 0)
            {
                if (idx >= o->object->childObjects().size())
                {
                    PyErr_SetString(PyExc_IndexError,
                                    QString("index out of range %1/%2")
                                    .arg(idx)
                                    .arg(o->object->childObjects().size())
                                    .toUtf8().constData());
                    return NULL;
                }
                return createBaseObjectInstance(
                            o->object->childObjects()[idx]);
            }
            auto list = PyList_New(0);
            if (!list)
            {
                PyErr_SetString(PyExc_MemoryError, "could not create list");
                return NULL;
            }
            for (auto c : o->object->childObjects())
            {
                auto pc = createBaseObjectInstance(c);
                if (0!=PyList_Append(list, pc))
                {
                    Py_DECREF(pc);
                    Py_DECREF(list);
                    PyErr_SetString(PyExc_MemoryError, "could not append to list");
                    return NULL;
                }
            }
            return list;
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

        // ----------------- setter ----------------

        static PyObject* set_name(PyObject* self, PyObject* arg)
        {
            char* cstr;
            if (!PyArg_Parse(arg, "s", &cstr))
                return NULL;
            QString str = QString::fromUtf8(cstr);
            MO__GETOBJ(o);
            if (o->editor)
                o->editor->setObjectName(o->object, str);
            else
                o->object->setName(str);

            Py_RETURN_NONE;
        }

        static bool insert_helper_(Object* parent, Object* newChild, int index)
        {
            if (!parent || !newChild)
            {
                PyErr_SetString(PyExc_TypeError, "NULL object");
                return false;
            }
            QString error;
            if (!parent->isSaveToAdd(newChild, error))
            {
                PyErr_SetString(PyExc_TypeError,
                                QString("can't add object '%1' to '%2': %3")
                                .arg(newChild->name())
                                .arg(parent->name())
                                .arg(error).toUtf8().constData());
                return false;
            }
            ObjectEditor::makeUniqueIds(parent->rootObject(), newChild);
            if (auto editor = parent->editor())
            {
                if (!editor->addObject(parent, newChild, index))
                {
                    PyErr_SetString(PyExc_RuntimeError,
                                    QString("could not add object '%1' to '%2'")
                                    .arg(newChild->name())
                                    .arg(parent->name()).toUtf8().constData());
                    return false;
                }
            }
            else
            {
                ObjectPrivate::addObject(parent, newChild, index);
            }
            return true;
        }

        static PyObject* add(PyObject* self, PyObject* args_)
        {
            PyObject * arg;
            int index = -1;
            if (!PyArg_ParseTuple(args_, "O|i", &arg, &index))
                return NULL;

            std::vector<BaseObjectStruct*> objects;
            // single object
            if (auto pother = BaseObject_cast(arg))
                objects.push_back(pother);
            // sequence
            else if (PySequence_Check(arg))
            {
                auto len = PySequence_Size(arg);
                for (Py_ssize_t i = 0; i < len; ++i)
                {
                    auto item = PySequence_GetItem(arg, i);
                    if (auto pother = BaseObject_cast(item))
                        objects.push_back(pother);
                    Py_XDECREF(item);
                }
            }
            else
            {
                PyErr_SetString(PyExc_TypeError,
                                "expected Object or sequence of Objects");
                return NULL;
            }
            if (objects.empty())
            {
                PyErr_SetString(PyExc_TypeError,
                                "no Object in sequence");
                return NULL;
            }

            for (auto pother : objects)
            {
                if (!pother->object)
                {
                    PyErr_SetString(PyExc_TypeError, "can't add NULL object");
                    return NULL;
                }
                MO__GETOBJ(pthis);
                if (!insert_helper_(pthis->object, pother->object, index))
                    return NULL;
                if (index >= 0)
                    ++index;
            }
            Py_INCREF(objects.back());
            return reinterpret_cast<PyObject*>(objects.back());
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

        { "num_children",
          (PyCFunction)BaseObjectFuncs::num_children,
          METH_NOARGS,
          "num_children() -> int\n"
          "Returns the number of children objects"
        },

        { "children",
          (PyCFunction)BaseObjectFuncs::children,
          METH_VARARGS,
          "children() -> list of Object\n"
          "children(int) -> Object\n"
          "Returns the list of all children or a specific children by index"
        },

        // --------- setter ---------

        { "set_name",
          (PyCFunction)BaseObjectFuncs::set_name,
          METH_O,
          "set_name(string) -> None\n"
          "Sets the user-defined name of the object"
        },

        { "add",
          (PyCFunction)BaseObjectFuncs::add,
          METH_VARARGS,
          "add(Object|Sequence) -> Object\n"
          "add(Object|Sequence, int) -> Object\n"
          "Adds the the object or the sequence of objects as children.\n"
          "If no index is given, the objects are appended, else they are inserted\n"
          "before the given index.\n"
          "The function returns the last added object\n"
        },

        { NULL, NULL, 0, NULL }
    };



    static PyTypeObject* BaseObject_type()
    {
        static PyTypeObject* type = 0;
        if (type == 0)
            type = new PyTypeObject(
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
            0,//BaseObjectFuncs::repr,     /*tp_repr*/
            0,                         /*tp_as_number*/
            0,                         /*tp_as_sequence*/
            0,                         /*tp_as_mapping*/
            0,                         /*tp_hash */
            0,                         /*tp_call*/
            BaseObjectFuncs::repr,     /*tp_str*/
            PyObject_GenericGetAttr,   /*tp_getattro*/
            PyObject_GenericSetAttr,   /*tp_setattro*/
            0,                         /*tp_as_buffer*/
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
            baseObjectDocString(),   /* tp_doc */
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
        });
        return type;
    }

    BaseObjectStruct* BaseObject_cast(PyObject* p)
    {
        if (PyObject_IsInstance(p, reinterpret_cast<PyObject*>(BaseObject_type())))
            return reinterpret_cast<BaseObjectStruct*>(p);
        else
            return nullptr;
    }

    PyObject* createBaseObjectInstance(Object*o)
    {
        auto pobj = PyObject_New(BaseObjectStruct, BaseObject_type());
        if (!pobj)
            return nullptr;
        pobj->object = o;
        if (o)
        {
            o->addRef();
            pobj->editor = o->editor();
        }
        return reinterpret_cast<PyObject*>(pobj);
    }

    /*
    XXX Not working yet
    PyTypeObject* createDerivedType(const QString& className)
    {
        auto type = new PyTypeObject;
        memcpy(type, BaseObject_type(), sizeof(PyTypeObject));

        type->tp_name = "matrixoptimizer.Derived";
        type->tp_base = BaseObject_type();

        if (0 != PyType_Ready(type))
            MO_ERROR("Failed to readify Derived wrapper with Python 3.4");

        type->tp_bases = Py_BuildValue("(O,)", (PyObject*)BaseObject_type());
        type->tp_mro = Py_BuildValue("(O,)", (PyObject*)BaseObject_type());
        //PyTuple_SetItem(type->tp_bases, 0, (PyObject*)BaseObject_type());

        return type;
    }
    */

} // extern "C"
} // namespace


void initObject(void* mod)
{
    static std::vector<PyTypeObject*> classes;
    if (!classes.empty())
        MO_ERROR("Duplicate call to PYTHON34::initObject()");

    PyObject* module = reinterpret_cast<PyObject*>(mod);

    initObjectType(module, BaseObject_type(), "Object");

    //classes.push_back(createDerivedType("SequenceFloat"));
    //initObjectType(module, classes.back(), "Derived");
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
