/** @file

    @brief

    <p>(c) 2016, stefan.berke@modular-audio-graphics.com</p>
    <p>All rights reserved</p>

    <p>created 2/29/2016</p>
*/

#ifdef MO_ENABLE_PYTHON34

#include <python3.5/Python.h>

#include <iostream>
#include "python_output.h"
#include "python.h"

namespace MO {
namespace PYTHON34 {

namespace {
extern "C" {

struct Python34Output
{
    PyObject_HEAD
    int fd;
    bool isError;
    PythonInterpreter* interpreter;
};

static PyObject*
printer_new(PyTypeObject *type, PyObject *, PyObject *)
{
    Python34Output* self;

    assert(type != NULL && type->tp_alloc != NULL);

    self = (Python34Output*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->fd = -1;
    }

    return (PyObject*)self;
}

static int
printer_init(PyObject *, PyObject *, PyObject *)
{
    PyErr_SetString(PyExc_TypeError,
                    "cannot create 'app_printer' instances");
    return -1;
}

static PyObject *
printer_write(Python34Output* self, PyObject *args)
{
    PyObject *unicode;
    PyObject *bytes = NULL;
    char *str;
    Py_ssize_t n;

    if (!PyArg_ParseTuple(args, "U", &unicode))
        return NULL;

    /* encode Unicode to UTF-8 */
    str = PyUnicode_AsUTF8AndSize(unicode, &n);
    if (str == NULL) {
        PyErr_Clear();
        bytes = _PyUnicode_AsUTF8String(unicode, "backslashreplace");
        if (bytes == NULL)
            return NULL;
        if (PyBytes_AsStringAndSize(bytes, &str, &n) < 0) {
            Py_DECREF(bytes);
            return NULL;
        }
        Py_XDECREF(bytes);
    }

    self->interpreter->write(str, self->isError);

#if 1
    if (self->isError)
        { std::cerr << str; std::cerr.flush(); }
    else
        { std::cout << str; std::cout.flush(); }
#endif

    return PyLong_FromSsize_t(n);
}

static PyObject *
printer_fileno(Python34Output *self)
{
    return PyLong_FromLong((long) self->fd);
}

static PyObject *
printer_repr(Python34Output *self)
{
    return PyUnicode_FromFormat("<app_printer(fd=%d) object at 0x%x>",
                                self->fd, self);
}

static PyObject *
printer_noop(Python34Output *)
{
    Py_RETURN_NONE;
}

static PyObject *
printer_isatty(Python34Output *)
{
    Py_RETURN_TRUE;
}


static PyMethodDef printer_methods[] = {
    {"close",           (PyCFunction)printer_noop, METH_NOARGS, ""},
    {"flush",           (PyCFunction)printer_noop, METH_NOARGS, ""},
    {"fileno",          (PyCFunction)printer_fileno, METH_NOARGS, ""},
    {"isatty",          (PyCFunction)printer_isatty, METH_NOARGS, ""},
    {"write",           (PyCFunction)printer_write, METH_VARARGS, ""},
    {NULL, NULL, 0, NULL }  /*sentinel */
};

static PyObject *
get_closed(Python34Output *, void *)
{
    Py_RETURN_FALSE;
}

static PyObject *
get_mode(Python34Output *, void *)
{
    return PyUnicode_FromString("w");
}

static PyObject *
get_encoding(Python34Output *, void *)
{
    Py_RETURN_NONE;
}

static PyGetSetDef printer_getsetlist[] = {
    { const_cast<char*>("closed"), (getter)get_closed, NULL,
      const_cast<char*>("True if the file is closed"), NULL },
    { const_cast<char*>("encoding"), (getter)get_encoding, NULL,
      const_cast<char*>("Encoding of the file"), NULL },
    { const_cast<char*>("mode"), (getter)get_mode, NULL,
      const_cast<char*>("String giving the file mode"), NULL },
    { NULL, NULL, NULL, NULL, NULL }
};

PyTypeObject Python34Output_Type = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "app_printer",                              /* tp_name */
    sizeof(Python34Output),                     /* tp_basicsize */
    0,                                          /* tp_itemsize */
    /* methods */
    0,                                          /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    (reprfunc)printer_repr,                     /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                         /* tp_flags */
    0,                                          /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    printer_methods,                            /* tp_methods */
    0,                                          /* tp_members */
    printer_getsetlist,                         /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    printer_init,                               /* tp_init */
    PyType_GenericAlloc,                        /* tp_alloc */
    printer_new,                                /* tp_new */
    PyObject_Del,                               /* tp_free */
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

} // extern "C"
} // namespace



void* createOutputObject(PythonInterpreter* inter, bool error)
{
    auto obj = PyObject_New(Python34Output, &Python34Output_Type);
    obj->fd = 42;
    obj->isError = error;
    obj->interpreter = inter;
    return obj;
}

} // namespace PYTHON34
} // namespace MO

#endif // MO_ENABLE_PYTHON34
