#include "../Packet.h"

PyObject* Packet_Get_Ethernet(Packet_Object* Self, void* Closure) {
    PyObject* Dict = PyDict_New();
    if (Dict == NULL) return NULL;
    
    char AddrBuffer[RTE_ETHER_ADDR_FMT_SIZE];
    struct rte_mbuf* Data = Self->Data;
    struct rte_ether_hdr* EthHeader = (struct rte_ether_hdr*)L2HeaderPtr;
    
    rte_ether_format_addr(AddrBuffer, RTE_ETHER_ADDR_FMT_SIZE, &(EthHeader->src_addr));
    if (PyDict_SetItemString(Dict, "Src-MAC", PyUnicode_FromString(AddrBuffer)))
        return NULL;
    
    rte_ether_format_addr(AddrBuffer, RTE_ETHER_ADDR_FMT_SIZE, &(EthHeader->dst_addr));
    if (PyDict_SetItemString(Dict, "Dst-MAC", PyUnicode_FromString(AddrBuffer)))
        return NULL;
    
    uint16_t EtherType = rte_be_to_cpu_16(EthHeader->ether_type);
    if (EtherType <= 0x0600) {
        if (PyDict_SetItemString(Dict, "Ether-type", PyUnicode_FromFormat("IEEE802.3 length: %d", (int)EtherType)))
            return NULL;
    }
    else{
        switch (EtherType) {
            case 0x0800:
                if (PyDict_SetItemString(Dict, "Ether-type", PyUnicode_FromString("Ethernet II Ipv4")))
                    return NULL;
                break;
            case 0x0806:
                if (PyDict_SetItemString(Dict, "Ether-type", PyUnicode_FromString("Ethernet II ARP")))
                    return NULL;
                break;
            case 0x86DD:
                if (PyDict_SetItemString(Dict, "Ether-type", PyUnicode_FromString("Ethernet II Ipv6")))
                    return NULL;
                break;
            default:
                //Unknown ether type value
                return NULL;
        }
    }
    return Dict;
}

int Packet_Set_Ethernet(Packet_Object* Self, PyObject* Value, void* Closure) {
    if (!PyDict_Check(Value))
        return -1;
    
    struct rte_mbuf* Data = Self->Data;
    struct rte_ether_hdr* EthHeader = (struct rte_ether_hdr*)L2HeaderPtr;
    PyObject* KeyValue = NULL;
    const char* StrBuffer = NULL;
    unsigned long EtherType = 0;
    
    KeyValue = PyDict_GetItemString(Value, "Src-MAC");
    if (KeyValue != NULL) {
        StrBuffer = PyUnicode_AsUTF8(KeyValue);
        if (unlikely(StrBuffer == NULL)) {
            PyErr_SetString(PyExc_TypeError, "Type of Src-MAC is not string.");
            return -1;
        }
        if (unlikely(rte_ether_unformat_addr(StrBuffer, &(EthHeader->src_addr)))) {
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
        if (unlikely(rte_ether_unformat_addr(StrBuffer, &(EthHeader->dst_addr)))) {
            PyErr_SetString(PyExc_ValueError, "Format of Dst-MAC is illeagal.");
            return -1;
        }
    }
    
    KeyValue = PyDict_GetItemString(Value, "Ether-Type");
    if (KeyValue != NULL) {
        EtherType = PyLong_AsUnsignedLong(KeyValue);
        if (unlikely(EtherType > 65535)) {
            PyErr_SetString(PyExc_ValueError, "Ethernet type should not be greater than 65535.");
            return -1;
        }
        EthHeader->ether_type = rte_cpu_to_be_16((uint16_t)EtherType);
    }
    return 0;
}
