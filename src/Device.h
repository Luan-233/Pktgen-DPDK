#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "Packet.h"
#include "PacketArray.h"

#include "./Utils/Config.h"

#include <rte_eal.h>
#include <rte_mempool.h>

#include <ctime>
#include <atomic>
#include <random>
#include <cstring>
#include <getopt.h>

extern "C"{
    typedef struct {
        PyObject_HEAD
        bool Start;
        struct EthConfigInfo Info;
        char MACAddress[RTE_ETHER_ADDR_FMT_SIZE];
    } Device_Object;
    
    void Device_Dealloc(Device_Object *Self);

    PyObject* Device_New(PyTypeObject *Type, PyObject *Args, PyObject *Kwds);
    
    int Device_Init(Device_Object* Self, PyObject *Args, PyObject *Kwds);
    
    PyObject* Device_Config(Device_Object* Self, PyObject* Py_UNUSED(ignored));
    
    PyObject* Device_GetMAC(Device_Object* Self, PyObject* Py_UNUSED(ignored));
    
    PyObject* Device_Receive(Device_Object* Self, PyObject *Args);
    PyObject* Device_Send(Device_Object* Self, PyObject *Args);
    PyObject* Device_SendWithDelay(Device_Object* Self, PyObject *Args);
    PyObject* Device_SendWithRate(Device_Object* Self, PyObject *Args);
    PyObject* Device_SendWithPossionDelay(Device_Object* Self, PyObject *Args);
    
    int Device_Type_Init();
}
