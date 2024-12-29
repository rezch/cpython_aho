#include "aho.h"

#define PY_SSIZE_T_CLEAN
#include <python3.8/Python.h>
#include <python3.8/structmember.h>

static const uint32_t DEFAULT_BUCKET_SIZE = 10;


struct {
    PyObject_HEAD
    dynamic_aho_t* daho;    /* dynamic daho struct */
    int daho_size;          /* buckets count */
} typedef DAhoObject;

static void
DAho_dealloc(DAhoObject *self)
{
    dynamic_aho_delete(self->daho);
    Py_XDECREF(self->daho);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

static PyMODINIT_FUNC
DAho_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    DAhoObject *self;
    self = (DAhoObject*) type->tp_alloc(type, 0);
    if (self == NULL)
        return NULL;

    self->daho_size = DEFAULT_BUCKET_SIZE;
    PyArg_ParseTuple(args, "|i", &self->daho_size);

    self->daho = dynamic_aho_init(self->daho_size);

    if (self->daho == NULL) {
        Py_DECREF(self);
        return NULL;
    }

    return (PyObject*) self;
}

static int
DAho_init(DAhoObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"daho_size", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &self->daho_size))
        return -1;

    return 0;
}

static PyMODINIT_FUNC
DAho_insert(DAhoObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"string", "count", NULL};

    char *str;
    int count = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|i", kwlist, &str, &count))
        return NULL;

    if (insert(self->daho, str, count) == -1) {
        PyErr_SetString(
            PyExc_RuntimeError,
            "some internal lib error on insert"
        );
    }

    Py_RETURN_NONE;
}

static PyMODINIT_FUNC
DAho_request(DAhoObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"text", NULL};

    char *str;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &str))
        return NULL;

    int32_t result = request(self->daho, str);
    if (result == -1) {
        PyErr_SetString(
            PyExc_RuntimeError,
            "some internal lib error on request"
        );
    }

    return Py_BuildValue("i", result);
}

static PyMODINIT_FUNC
DAho_resize(DAhoObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"size", NULL};

    int new_size;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "i", kwlist, &new_size))
        return NULL;

    if (new_size < 0)
        return NULL;

    if (resize(self->daho, new_size) == -1) {
        PyErr_SetString(
            PyExc_MemoryError,
            "cannot resize structure"
        );
    }

    self->daho_size = new_size;

    Py_RETURN_NONE;
}

static PyMethodDef
DAho_methods[] = {
    {
        .ml_name    =   "insert",
        .ml_meth    =   (PyCFunction) DAho_insert,
        .ml_flags   =   METH_VARARGS | METH_KEYWORDS,
        .ml_doc     =   "insert string to aho set"
    },
    {
        .ml_name    =   "request",
        .ml_meth    =   (PyCFunction) DAho_request,
        .ml_flags   =   METH_VARARGS | METH_KEYWORDS,
        .ml_doc     =   "count the inclusions of strings from the aho set in a text"
    },
    {
        .ml_name    =   "resize",
        .ml_meth    =   (PyCFunction) DAho_resize,
        .ml_flags   =   METH_VARARGS | METH_KEYWORDS,
        .ml_doc     =   "change aho buckets count"
    },
    { NULL }    /* Sentinel */
};

static PyMemberDef
DAho_members[] = {
    {
        .name       =   "buckets",
        .type       =   T_INT,
        .offset     =   offsetof(DAhoObject, daho_size),
        .flags      =   0,
        .doc        =   "daho buckets count"
    },
    { NULL }    /* Sentinel */
};

static PyTypeObject
DAhoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name        =   "daho.DAho",
    .tp_doc         =   "Dynamic daho",
    .tp_basicsize   =   sizeof(DAhoObject),
    .tp_itemsize    =   0,
    .tp_flags       =   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new         =   DAho_new,
    .tp_init        =   (initproc) DAho_init,
    .tp_dealloc     =   (destructor) DAho_dealloc,
    .tp_members     =   DAho_members,
    .tp_methods     =   DAho_methods,
};

static PyModuleDef
dahomodule = {
    PyModuleDef_HEAD_INIT,
    .m_name         =   "daho",
    .m_doc          =   "Dynamic daho",
    .m_size         =   -1,
};

PyMODINIT_FUNC
PyInit_daho(void)
{
    PyObject *module;
    if (PyType_Ready(&DAhoType) < 0)
        return NULL;

    module = PyModule_Create(&dahomodule);
    if (module == NULL)
        return NULL;

    Py_INCREF(&DAhoType);
    if (PyModule_AddObject(module, "DAho", (PyObject *) &DAhoType) < 0) {
        Py_DECREF(&DAhoType);
        Py_DECREF(module);
        return NULL;
    }

    return module;
}
