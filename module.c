#include "aho.h"

#define PY_SSIZE_T_CLEAN
#include <python3.8/Python.h>
#include "structmember.h"


struct {
    PyObject_HEAD
    dynamic_aho_t* daho; /* dynamic daho */
    int daho_size;       /* daho buckets count */
} typedef DAhoObject;

static void
DAho_dealloc(DAhoObject *self)
{
    dynamic_aho_delete(self->daho);
    Py_XDECREF(self->daho);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
DAho_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    DAhoObject *self;
    self = (DAhoObject *) type->tp_alloc(type, 0);

    if (self == NULL)
        return NULL;

    self->daho_size = 31;
    PyArg_ParseTuple(args, "|i", &self->daho_size);

    self->daho = dynamic_aho_init(self->daho_size);

    if (self->daho == NULL) {
        Py_DECREF(self);
        return NULL;
    }

    return (PyObject *) self;
}

static int
DAho_init(DAhoObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"daho_size", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, &self->daho_size))
        return -1;
    return 0;
}

static PyMemberDef DAho_members[] = {
    {"buckets", T_INT, offsetof(DAhoObject, daho_size), 0,
     "daho buckets count"},
    {NULL}  /* Sentinel */
};

static PyObject *
DAho_insert(DAhoObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"string", "count", NULL};

    char *str;
    int count = 1;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|i", kwlist, &str, &count))
        return NULL;

    insert(self->daho, str, count);

    Py_RETURN_NONE;
}

static PyObject *
DAho_request(DAhoObject *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"text", NULL};

    char *str;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "s|", kwlist, &str))
        return NULL;

    return Py_BuildValue(
            "i",
            request(self->daho, str)
    );
}

static PyMethodDef DAho_methods[] = {
    {"insert", (PyCFunction) DAho_insert, METH_VARARGS | METH_KEYWORDS,
     "insert string to aho set"
    },
    {"request", (PyCFunction) DAho_request, METH_VARARGS | METH_KEYWORDS,
     "count the inclusions of strings from the aho set in a text"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject DAhoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "daho.DAho",
    .tp_doc = "Dynamic daho",
    .tp_basicsize = sizeof(DAhoObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = DAho_new,
    .tp_init = (initproc) DAho_init,
    .tp_dealloc = (destructor) DAho_dealloc,
    .tp_members = DAho_members,
    .tp_methods = DAho_methods,
};

static PyModuleDef dahomodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "daho",
    .m_doc = "Dynamic daho",
    .m_size = -1,
};

PyMODINIT_FUNC
PyInit_daho(void)
{
    PyObject *m;
    if (PyType_Ready(&DAhoType) < 0)
        return NULL;

    m = PyModule_Create(&dahomodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&DAhoType);
    if (PyModule_AddObject(m, "DAho", (PyObject *) &DAhoType) < 0) {
        Py_DECREF(&DAhoType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}
