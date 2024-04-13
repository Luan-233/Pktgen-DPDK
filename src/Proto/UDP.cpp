#include "../Packet.h"

PyObject* Packet_Get_UDP(Packet_Object* Self, void* Closure) {
    PyObject* Dict = PyDict_New();
    if (Dict == NULL) return NULL;
    
    struct rte_mbuf* Data = Self->Data;
    struct rte_udp_hdr* UDPHeader = (struct rte_udp_hdr*)L4HeaderPtr;
    
    if (PyDict_SetItemString(Dict, "Src-Port", PyLong_FromLong((long)rte_be_to_cpu_16(UDPHeader->src_port))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Dst-Port", PyLong_FromLong((long)rte_be_to_cpu_16(UDPHeader->dst_port))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Length", PyLong_FromLong((long)rte_be_to_cpu_16(UDPHeader->dgram_len))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Check-Sum", PyLong_FromLong((long)rte_be_to_cpu_16(UDPHeader->dgram_cksum))))
        return NULL;
    
    return Dict;
}

int Packet_Set_UDP(Packet_Object* Self, PyObject* Value, void* Closure) {
    if (!PyDict_Check(Value))
        return -1;
        
    struct rte_mbuf* Data = Self->Data;
    struct rte_udp_hdr* UDPHeader = (struct rte_udp_hdr*)L4HeaderPtr;
    PyObject* KeyValue = NULL;
    unsigned long Port = 0;
    
    KeyValue = PyDict_GetItemString(Value, "Src-Port");
    if (KeyValue != NULL) {
        Port = PyLong_AsUnsignedLong(KeyValue);
        if (unlikely(Port > 65535)) {
            PyErr_SetString(PyExc_ValueError, "UDP Src Port should not be greater than 65535.");
            return -1;
        }
        UDPHeader->src_port = rte_cpu_to_be_16((uint16_t)Port);
    }
    
    KeyValue = PyDict_GetItemString(Value, "Dst-Port");
    if (KeyValue != NULL) {
        Port = PyLong_AsUnsignedLong(KeyValue);
        if (unlikely(Port > 65535)) {
            PyErr_SetString(PyExc_ValueError, "UDP Dsc Port should not be greater than 65535.");
            return -1;
        }
        UDPHeader->dst_port = rte_cpu_to_be_16((uint16_t)Port);
    }
    
    return 0;
}
