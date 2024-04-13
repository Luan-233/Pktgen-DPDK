#include "../Packet.h"

PyObject* Packet_Get_ARP(Packet_Object* Self, void* Closure) {
    PyObject* Dict = PyDict_New();
    if (Dict == NULL) return NULL;
    
    char IPv4Addr[IPV4_ADDR_FMT_SIZE];
    char EtherAddr[RTE_ETHER_ADDR_FMT_SIZE];
    struct rte_mbuf* Data = Self->Data;
    struct rte_arp_hdr* ARPHeader = (struct rte_arp_hdr*)(L3HeaderPtr - sizeof(struct rte_arp_hdr));
    
    ParseInteger2IPv4(rte_be_to_cpu_32(ARPHeader->arp_data.arp_sip), IPv4Addr);
    if (PyDict_SetItemString(Dict, "Src-IP", PyUnicode_FromString(IPv4Addr)))
        return NULL;
    
    ParseInteger2IPv4(rte_be_to_cpu_32(ARPHeader->arp_data.arp_tip), IPv4Addr);
    if (PyDict_SetItemString(Dict, "Dst-IP", PyUnicode_FromString(IPv4Addr)))
        return NULL;
        
    rte_ether_format_addr(EtherAddr, RTE_ETHER_ADDR_FMT_SIZE, &(ARPHeader->arp_data.arp_sha));
    if (PyDict_SetItemString(Dict, "Src-MAC", PyUnicode_FromString(EtherAddr)))
        return NULL;
        
    rte_ether_format_addr(EtherAddr, RTE_ETHER_ADDR_FMT_SIZE, &(ARPHeader->arp_data.arp_tha));
    if (PyDict_SetItemString(Dict, "Dst-MAC", PyUnicode_FromString(EtherAddr)))
        return NULL;
    
    if (PyDict_SetItemString(Dict, "Op-Code", PyLong_FromLong((long)rte_be_to_cpu_16(ARPHeader->arp_opcode))))
        return NULL;
    
    return Dict;
}

int Packet_Set_ARP(Packet_Object* Self, PyObject* Value, void* Closure) {
    if (!PyDict_Check(Value))
        return -1;
    
    struct rte_mbuf* Data = Self->Data;
    struct rte_arp_hdr* ARPHeader = (struct rte_arp_hdr*)(L2HeaderPtr + sizeof(struct rte_ether_hdr));
    PyObject* KeyValue = NULL;
    const char* StrBuffer = NULL;
    uint32_t IPv4Addr = 0;
    unsigned long OpCode = 0;
    
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
        ARPHeader->arp_data.arp_sip = rte_cpu_to_be_32(IPv4Addr);
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
        ARPHeader->arp_data.arp_tip = rte_cpu_to_be_32(IPv4Addr);
    }
    
    KeyValue = PyDict_GetItemString(Value, "Src-MAC");
    if (KeyValue != NULL) {
        StrBuffer = PyUnicode_AsUTF8(KeyValue);
        if (unlikely(StrBuffer == NULL)) {
            PyErr_SetString(PyExc_TypeError, "Type of Src-MAC is not string.");
            return -1;
        }
        if (unlikely(rte_ether_unformat_addr(StrBuffer, &(ARPHeader->arp_data.arp_sha)))) {
            PyErr_SetString(PyExc_ValueError, "Format of Src-MAC is illeagal.");
            return -1;
        }
    }
    
    KeyValue = PyDict_GetItemString(Value, "Dst-MAC");
    if (KeyValue != NULL) {
        StrBuffer = PyUnicode_AsUTF8(KeyValue);
        if (unlikely(StrBuffer == NULL)) {
            PyErr_SetString(PyExc_TypeError, "Type of Dst-MAC is not string.");
            return -1;
        }
        if (unlikely(rte_ether_unformat_addr(StrBuffer, &(ARPHeader->arp_data.arp_tha)))) {
            PyErr_SetString(PyExc_ValueError, "Format of Dst-MAC is illeagal.");
            return -1;
        }
    }
    
    KeyValue = PyDict_GetItemString(Value, "Op-Code");
    if (KeyValue != NULL) {
        OpCode = PyLong_AsUnsignedLong(KeyValue);
        if (unlikely(OpCode > 65535)) {
            PyErr_SetString(PyExc_ValueError, "ARP opcode should not be greater than 65535.");
            return -1;
        }
        ARPHeader->arp_opcode = rte_cpu_to_be_16((uint16_t)OpCode);
    }
    return 0; 
}
