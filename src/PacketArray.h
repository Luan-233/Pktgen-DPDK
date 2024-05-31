#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "Device.h"
#include "Packet.h"
#include "MemPool.h"

#include "./Utils/Header.h"

#include <rte_ip.h>
#include <rte_arp.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_ether.h>
#include <rte_memcpy.h>
#include <rte_mempool.h>
#include <rte_branch_prediction.h>

#include <atomic> 
#include <getopt.h>
#include <iostream> 

extern "C"{
    typedef struct {
        PyObject_HEAD
        uint16_t Length;
        uint16_t AvilLength;
        struct rte_mbuf** Pkts;
    } PacketArray_Object;
    
    void PacketArray_Dealloc(PacketArray_Object *Self);

    PyObject* PacketArray_New(PyTypeObject *Type, PyObject *Args, PyObject *Kwds);
    
    int PacketArray_Init(PacketArray_Object* Self, PyObject *Args, PyObject *Kwds);
    
    PyObject* PacketArray_Fill(PacketArray_Object* Self, PyObject *Args, PyObject *Kwds);
    
    Py_ssize_t PacketArray_Length(PacketArray_Object* PktArray);
    PyObject* PacketArray_Concat(PacketArray_Object *Object1, PacketArray_Object* Object2);
    PyObject* PacketArray_Repeat(PacketArray_Object *Object, Py_ssize_t Count);
    PyObject* PacketArray_InPlaceConcat(PacketArray_Object *Object1, PacketArray_Object* Object2);
    PyObject* PacketArray_InPlaceRepeat(PacketArray_Object *Object, Py_ssize_t Count);
    PyObject* PacketArray_GetItem(PacketArray_Object* PktArray, Py_ssize_t Index);
    
    int PacketArray_Type_Init(); 
}
