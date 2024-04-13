#include "MemPool.h"

extern "C" {
    static PyMemberDef MemPool_Members[] = {
        {NULL}
    };
    
    static PyMethodDef MemPool_Methods[] = {
        {NULL}
    };
    
    static PyGetSetDef MemPool_GetSetters[] = {
        {NULL}
    };
    
    PyTypeObject MemPool_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
    };
}

void MemPool_Dealloc(MemPool_Object *Self) {
    Py_TYPE(Self)->tp_free((PyObject*)Self);
}

PyObject* MemPool_New(PyTypeObject *Type, PyObject *Args, PyObject *Kwds) {
    MemPool_Object* Self;
    Self = (MemPool_Object*)Type->tp_alloc(Type, 0);
    if (Self != NULL)
        Self->MemPoolPtr = NULL;
    return (PyObject*) Self;
}

int MemPool_Init(MemPool_Object* Self, PyObject *Args, PyObject *Kwds){
    static std::atomic<int> Counter(0);
    unsigned int PoolSize = 0;
    if (!PyArg_ParseTuple(Args, "I", &PoolSize)) return -1;
    Self->MemPoolPtr = MemPoolAlloc(PoolSize);
    if (Self->MemPoolPtr == NULL) {
        PyErr_SetString(PyExc_MemoryError, "Cannot init MemPool. Please check if size is valid.");
        return -1;
    }
    else Self->MemPoolSize = PoolSize;
    return 0;
}

int MemPool_Type_Init() {
    MemPool_Type.tp_name = "Pktgen_DPDK.MemPool";
    MemPool_Type.tp_doc = PyDoc_STR("Memory pool objects.");
    MemPool_Type.tp_basicsize = sizeof(MemPool_Object);
    MemPool_Type.tp_itemsize = 0;
    MemPool_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    MemPool_Type.tp_new = MemPool_New;
    MemPool_Type.tp_init = (initproc)MemPool_Init;
    MemPool_Type.tp_dealloc = (destructor)MemPool_Dealloc;
    MemPool_Type.tp_members = MemPool_Members;
    MemPool_Type.tp_methods = MemPool_Methods;
    MemPool_Type.tp_getset = MemPool_GetSetters;
    return PyType_Ready(&MemPool_Type);
}
