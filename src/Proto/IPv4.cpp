#include "../Packet.h"

#define OffsetMask ((1 << 13) - 1)

PyObject* Packet_Get_IPv4(Packet_Object* Self, void* Closure) {
    PyObject* Dict = PyDict_New();
    if (Dict == NULL) return NULL;
    
    char AddrBuffer[IPV4_ADDR_FMT_SIZE]; 
    struct rte_mbuf* Data = Self->Data;
    struct rte_ipv4_hdr* IPv4Header = (struct rte_ipv4_hdr*)L3HeaderPtr;
    
    ParseInteger2IPv4(rte_be_to_cpu_32(IPv4Header->src_addr), AddrBuffer);
    if (PyDict_SetItemString(Dict, "Src-IP", PyUnicode_FromString(AddrBuffer)))
        return NULL;
    
    ParseInteger2IPv4(rte_be_to_cpu_32(IPv4Header->dst_addr), AddrBuffer);
    if (PyDict_SetItemString(Dict, "Dst-IP", PyUnicode_FromString(AddrBuffer)))
        return NULL;
    
    if (PyDict_SetItemString(Dict, "Length", PyLong_FromLong((long)rte_be_to_cpu_16(IPv4Header->total_length))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Packet-ID", PyLong_FromLong((long)rte_be_to_cpu_16(IPv4Header->packet_id))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Offset", PyLong_FromLong((long)rte_be_to_cpu_16(IPv4Header->fragment_offset & OffsetMask))))
        return NULL;
    if (PyDict_SetItemString(Dict, "TTL", PyLong_FromLong((long)(IPv4Header->time_to_live))))
        return NULL;
        
    const char* Proto = GetIPProto(IPv4Header->next_proto_id);
    if (Proto == NULL) {
        if (PyDict_SetItemString(Dict, "Proto", PyUnicode_FromString("Unknown")))
            return NULL;
    }
    else {
        if (PyDict_SetItemString(Dict, "Proto", PyUnicode_FromString(Proto)))
            return NULL;
    }
    
    if (PyDict_SetItemString(Dict, "Check-Sum", PyLong_FromLong((long)(IPv4Header->hdr_checksum))))
        return NULL;
    
    //TODO: add DF MF identifier
    return Dict;
}

int Packet_Set_IPv4(Packet_Object* Self, PyObject* Value, void* Closure) {
    if (!PyDict_Check(Value))
        return -1;
        
    struct rte_mbuf* Data = Self->Data;
    struct rte_ipv4_hdr* IPv4Header = (struct rte_ipv4_hdr*)L3HeaderPtr;
    PyObject* KeyValue = NULL;
    const char* StrBuffer = NULL;
    uint32_t IPv4Addr = 0;
    
    KeyValue = PyDict_GetItemString(Value, "Src-IP");
    if (KeyValue != NULL) {
        StrBuffer = PyUnicode_AsUTF8(KeyValue);
        if (unlikely(StrBuffer == NULL)) {
            PyErr_SetString(PyExc_TypeError, "Type of Src-IP is not string.");
            return -1;
        }
        if (unlikely(ParseIPv42Integer(StrBuffer, &IPv4Addr))) {
            PyErr_SetString(PyExc_ValueError, "Format of Src-IP is illeagal.");
            return -1;
        }
        IPv4Header->src_addr = rte_cpu_to_be_32(IPv4Addr);
    }
    
    KeyValue = PyDict_GetItemString(Value, "Dst-IP");
    if (KeyValue != NULL) {
        StrBuffer = PyUnicode_AsUTF8(KeyValue);
        if (unlikely(StrBuffer == NULL)) {
            PyErr_SetString(PyExc_TypeError, "Type of Dst-IP is not string.");
            return -1;
        }
        if (unlikely(ParseIPv42Integer(StrBuffer, &IPv4Addr))) {
            PyErr_SetString(PyExc_ValueError, "Format of Dst-IP is illeagal.");
            return -1;
        }
        IPv4Header->dst_addr = rte_cpu_to_be_32(IPv4Addr);
    }
    
    return 0;
}
