#include "../Packet.h"

#define FlowLabelMask ((1ll << 20) - 1ll)

PyObject* Packet_Get_IPv6(Packet_Object* Self, void* Closure) {
    PyObject* Dict = PyDict_New();
    if (Dict == NULL) return NULL;
    
    char AddrBuffer[IPV4_ADDR_FMT_SIZE];
    struct rte_mbuf* Data = Self->Data;
    struct rte_ipv6_hdr* IPv6Header = (struct rte_ipv6_hdr*)L3HeaderPtr;
    
    ParseInteger2IPv6(IPv6Header->src_addr, AddrBuffer);
    if (PyDict_SetItemString(Dict, "Src-IP", PyUnicode_FromString(AddrBuffer)))
        return NULL;
    
    ParseInteger2IPv6(IPv6Header->dst_addr, AddrBuffer);
    if (PyDict_SetItemString(Dict, "Dst-IP", PyUnicode_FromString(AddrBuffer)))
        return NULL;
    
    if (PyDict_SetItemString(Dict, "Flow-Label", PyLong_FromLong((long)rte_be_to_cpu_16(IPv6Header->vtc_flow & FlowLabelMask))))
        return NULL;
    if (PyDict_SetItemString(Dict, "Length", PyLong_FromLong((long)rte_be_to_cpu_16(IPv6Header->payload_len))))
        return NULL;
    
    const char* Proto = GetIPProto(IPv6Header->proto);
    if (Proto == NULL) {
        if (PyDict_SetItemString(Dict, "Proto", PyUnicode_FromString("Unknown")))
            return NULL;
    }
    else {
        if (PyDict_SetItemString(Dict, "Proto", PyUnicode_FromString(Proto)))
            return NULL;
    }
    
    if (PyDict_SetItemString(Dict, "TTL", PyLong_FromLong((long)(IPv6Header->hop_limits))))
        return NULL;
    
    return Dict;
}

int Packet_Set_IPv6(Packet_Object* Self, PyObject* Value, void* Closure) {
    if (!PyDict_Check(Value))
        return -1;
        
    struct rte_mbuf* Data = Self->Data;
    struct rte_ipv6_hdr* IPv6Header = (struct rte_ipv6_hdr*)L3HeaderPtr;
    PyObject* KeyValue = NULL;
    const char* StrBuffer = NULL;
    uint8_t IPv6Addr[16];
    
    KeyValue = PyDict_GetItemString(Value, "Src-IP");
    if (KeyValue != NULL) {
        StrBuffer = PyUnicode_AsUTF8(KeyValue);
        if (unlikely(StrBuffer == NULL)) {
            PyErr_SetString(PyExc_TypeError, "Type of Src-IP is not string.");
            return -1;
        }
        if (unlikely(ParseIPv62Integer(StrBuffer, IPv6Addr))) {
            PyErr_SetString(PyExc_ValueError, "Format of Src-IP is illeagal.");
            return -1;
        }
        rte_memcpy(IPv6Header->src_addr, IPv6Addr, sizeof(uint8_t) * 16);
    }
    
    KeyValue = PyDict_GetItemString(Value, "Dst-IP");
    if (KeyValue != NULL) {
        StrBuffer = PyUnicode_AsUTF8(KeyValue);
        if (unlikely(StrBuffer == NULL)) {
            PyErr_SetString(PyExc_TypeError, "Type of Dst-IP is not string.");
            return -1;
        }
        if (unlikely(ParseIPv62Integer(StrBuffer, IPv6Addr))) {
            PyErr_SetString(PyExc_ValueError, "Format of Dst-IP is illeagal.");
            return -1;
        }
        rte_memcpy(IPv6Header->dst_addr, IPv6Addr, sizeof(uint8_t) * 16);
    }
    
    return 0;
}
