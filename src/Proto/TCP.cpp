#include "../Packet.h"

PyObject* Packet_Get_TCP(Packet_Object* Self, void* Closure) {
    PyObject* Dict = PyDict_New();
    if (Dict == NULL) return NULL;
    
    char FlagBuffer[1 << 5];
    struct rte_mbuf* Data = Self->Data;
    struct rte_tcp_hdr* TCPHeader = (struct rte_tcp_hdr*)L4HeaderPtr;
    
    if (PyDict_SetItemString(Dict, "Src-Port", PyLong_FromLong((long)rte_be_to_cpu_16(TCPHeader->src_port))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Dst-Port", PyLong_FromLong((long)rte_be_to_cpu_16(TCPHeader->dst_port))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Seq", PyLong_FromLong((long)rte_be_to_cpu_32(TCPHeader->sent_seq))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Ack", PyLong_FromLong((long)rte_be_to_cpu_32(TCPHeader->recv_ack))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Window", PyLong_FromLong((long)rte_be_to_cpu_16(TCPHeader->rx_win))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Check-Sum", PyLong_FromLong((long)rte_be_to_cpu_16(TCPHeader->cksum))))
        return NULL;
        
    GetTCPFlags(TCPHeader->tcp_flags, FlagBuffer);
    if (PyDict_SetItemString(Dict, "Flags", PyUnicode_FromString(FlagBuffer)))
        return NULL;
    
    return Dict;
}

int Packet_Set_TCP(Packet_Object* Self, PyObject* Value, void* Closure) {
    if (!PyDict_Check(Value))
        return -1;
        
    struct rte_mbuf* Data = Self->Data;
    struct rte_tcp_hdr* TCPHeader = (struct rte_tcp_hdr*)L4HeaderPtr;
    PyObject* KeyValue = NULL;
    unsigned long Port = 0;
    
    KeyValue = PyDict_GetItemString(Value, "Src-Port");
    if (KeyValue != NULL) {
        Port = PyLong_AsUnsignedLong(KeyValue);
        if (unlikely(Port > 65535)) {
            PyErr_SetString(PyExc_ValueError, "TCP Src Port should not be greater than 65535.");
            return -1;
        }
        TCPHeader->src_port = rte_cpu_to_be_16((uint16_t)Port);
    }
    
    KeyValue = PyDict_GetItemString(Value, "Dst-Port");
    if (KeyValue != NULL) {
        Port = PyLong_AsUnsignedLong(KeyValue);
        if (unlikely(Port > 65535)) {
            PyErr_SetString(PyExc_ValueError, "TCP Dst Port should not be greater than 65535.");
            return -1;
        }
        TCPHeader->dst_port = rte_cpu_to_be_16((uint16_t)Port);
    }
    
    return 0;
}
