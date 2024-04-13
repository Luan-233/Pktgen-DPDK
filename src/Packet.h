#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "Device.h"

#include "./Utils/Format.h"
#include "./Utils/Protocols.h"

#include <rte_ip.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_arp.h>
#include <rte_ether.h>
#include <rte_mempool.h>
#include <rte_mbuf_core.h>
#include <rte_mbuf_ptype.h>
#include <rte_branch_prediction.h> 

#include <cstring>
#include <getopt.h>

#define L2HeaderPtr (rte_pktmbuf_mtod(Data, uint8_t*))
#define L3HeaderPtr (rte_pktmbuf_mtod(Data, uint8_t*) + Data->l2_len)
#define L4HeaderPtr (rte_pktmbuf_mtod(Data, uint8_t*) + Data->l2_len + Data->l3_len)
#define DataPtr (rte_pktmbuf_mtod(Data, uint8_t*) + Data->l2_len + Data->l3_len + Data->l4_len)

extern "C"{
    typedef struct {
        PyObject_HEAD
        struct rte_mbuf* Data;
    } Packet_Object;
    
    void Packet_Dealloc(Packet_Object *Self); 

    PyObject* Packet_New(PyTypeObject *Type, PyObject *Args, PyObject *Kwds);
    
    int Packet_Init(Packet_Object* Self, PyObject *Args, PyObject *Kwds);
    
    PyObject* Packet_Show(Packet_Object* Self, PyObject* Py_UNUSED(ignored));
    
    PyObject* Packet_TimeStamp(Packet_Object* Self, PyObject* Py_UNUSED(ignored));
    
    PyObject* Packet_CalcL3CkSum(Packet_Object* Self, PyObject* Py_UNUSED(ignored));
    
    PyObject* Packet_CalcL4CkSum(Packet_Object* Self, PyObject* Py_UNUSED(ignored));
    
    PyObject* Packet_Get_Ethernet(Packet_Object* Self, void* Closure);
    int Packet_Set_Ethernet(Packet_Object* Self, PyObject* Value, void* Closure);
    
    PyObject* Packet_Get_IPv4(Packet_Object* Self, void* Closure);
    int Packet_Set_IPv4(Packet_Object* Self, PyObject* Value, void* Closure);
    
    PyObject* Packet_Get_ARP(Packet_Object* Self, void* Closure);
    int Packet_Set_ARP(Packet_Object* Self, PyObject* Value, void* Closure);
    
    PyObject* Packet_Get_IPv6(Packet_Object* Self, void* Closure);
    int Packet_Set_IPv6(Packet_Object* Self, PyObject* Value, void* Closure);
    
    PyObject* Packet_Get_TCP(Packet_Object* Self, void* Closure);
    int Packet_Set_TCP(Packet_Object* Self, PyObject* Value, void* Closure);
    
    PyObject* Packet_Get_UDP(Packet_Object* Self, void* Closure);
    int Packet_Set_UDP(Packet_Object* Self, PyObject* Value, void* Closure);
    
    int Packet_Type_Init();
}
