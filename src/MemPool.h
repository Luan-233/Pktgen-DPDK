#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "./Utils/Memory.h" 

#include <rte_mbuf.h>
#include <rte_lcore.h>
#include <rte_common.h>
#include <rte_mempool.h>

#include <stdio.h>
#include <getopt.h>

extern "C"{
    typedef struct {
        PyObject_HEAD
        uint32_t MemPoolSize;
        struct rte_mempool* MemPoolPtr;
    } MemPool_Object;
    
    void MemPool_Dealloc(MemPool_Object *Self);

    PyObject* MemPool_New(PyTypeObject *Type, PyObject *args, PyObject *kwds);
    
    int MemPool_Init(MemPool_Object* Self, PyObject *Args, PyObject *Kwds);
    
    int MemPool_Type_Init();
}
