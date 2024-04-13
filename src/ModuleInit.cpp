#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "Device.h"
#include "Packet.h"
#include "MemPool.h"
#include "PacketArray.h"

#include <rte_eal.h>

extern PyTypeObject Device_Type;
extern PyTypeObject Packet_Type;
extern PyTypeObject MemPool_Type;
extern PyTypeObject PacketArray_Type;

extern "C" {
    PyModuleDef LGen_ModuleDef = {
        PyModuleDef_HEAD_INIT,
        .m_name = "Pktgen_DPDK",
        .m_doc = "A Packet generator powered by DPDK.",
        .m_size = -1,
    };
}

PyMODINIT_FUNC PyInit_Pktgen_DPDK(void) {
    int Argc = 0;
    wchar_t** Argv = NULL;
    Py_GetArgcArgv(&Argc, &Argv);
    rte_eal_init(Argc, (char**)Argv);
    
    PyObject* ModulePtr;
    
    if (Device_Type_Init() < 0)
        return NULL;
    if (Packet_Type_Init() < 0)
        return NULL;
    if (MemPool_Type_Init() < 0)
        return NULL;
    if (PacketArray_Type_Init() < 0)
        return NULL;
        
    printf("Finish init type meta data.\n");

    ModulePtr = PyModule_Create(&LGen_ModuleDef);
    if (ModulePtr == NULL)
        return NULL;

    Py_INCREF(&Device_Type);
    if (PyModule_AddObject(ModulePtr, "Device", (PyObject*)&Device_Type) < 0) {
        Py_DECREF(&Device_Type), Py_DECREF(ModulePtr);
        return NULL;
    }
    
    Py_INCREF(&Packet_Type);
    if (PyModule_AddObject(ModulePtr, "Packet", (PyObject*)&Packet_Type) < 0) {
        Py_DECREF(&Packet_Type), Py_DECREF(ModulePtr);
        return NULL;
    }
    
    Py_INCREF(&MemPool_Type);
    if (PyModule_AddObjectRef(ModulePtr, "MemPool", (PyObject*)&MemPool_Type) < 0) {
        Py_DECREF(&MemPool_Type), Py_DECREF(ModulePtr);
        return NULL;
    }
    
    Py_INCREF(&PacketArray_Type);
    if (PyModule_AddObject(ModulePtr, "PacketArray", (PyObject*)&PacketArray_Type) < 0) {
        Py_DECREF(&PacketArray_Type), Py_DECREF(ModulePtr);
        return NULL;
    }

    return ModulePtr;
}
