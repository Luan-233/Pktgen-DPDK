#include "PacketArray.h"

extern PyTypeObject Packet_Type;
extern PyTypeObject MemPool_Type;

std::atomic<int> __Builtin_ArrayCounter(0);

extern "C" {
    static PyMemberDef PacketArray_Members[] = { {
            .name = "Length",
            .type = T_SHORT,
            .offset = offsetof(PacketArray_Object, Length),
            .flags = READONLY,
            .doc = "Length of packet array."
        },{
            .name = "AvilableLength",
            .type = T_SHORT,
            .offset = offsetof(PacketArray_Object, AvilLength),
            .flags = READONLY,
            .doc = "Length of packet array."
        },
        {NULL}
    };
    
    static PyMethodDef PacketArray_Methods[] = { {
            .ml_name = "Fill",
            .ml_meth = (PyCFunction)PacketArray_Fill,
            .ml_flags = METH_VARARGS | METH_KEYWORDS,
            .ml_doc = "Fill an array with specified infomation."
        },
        {NULL}
    };
    
    static PyGetSetDef PacketArray_GetSetters[] = {
        {NULL}
    };
    
    static PySequenceMethods PacketArray_SeqMethods = {
        .sq_length = (lenfunc)PacketArray_Length,
        .sq_concat = (binaryfunc)PacketArray_Concat,
        .sq_repeat = (ssizeargfunc)PacketArray_Repeat,
        .sq_item = (ssizeargfunc)PacketArray_GetItem,
        .sq_ass_item = NULL,
        .sq_contains = NULL,
        .sq_inplace_concat = (binaryfunc)PacketArray_InPlaceConcat,
        .sq_inplace_repeat = (ssizeargfunc)PacketArray_InPlaceRepeat,
    };
    
    PyTypeObject PacketArray_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
    };
}

void PacketArray_Dealloc(PacketArray_Object *Self) {
    rte_pktmbuf_free_bulk(Self->Pkts, Self->Length);
    delete [] Self->Pkts;
    --__Builtin_ArrayCounter;
    Py_TYPE(Self)->tp_free((PyObject*)Self);
}

PyObject* PacketArray_New(PyTypeObject *Type, PyObject *Args, PyObject *Kwds) {
    PacketArray_Object* Self;
    Self = (PacketArray_Object*)Type->tp_alloc(Type, 0);
    if (Self != NULL)
        Self->Pkts = NULL;
    ++__Builtin_ArrayCounter;
    return (PyObject*)Self;
}

int PacketArray_Init(PacketArray_Object* Self, PyObject *Args, PyObject *Kwds) {
    static char* KwdList[] = {"Length", "MemPool", "L2", "L3", "L4", "DataLen", NULL};
    
    const char *L2 = NULL, *L3 = NULL, *L4 = NULL;
    uint16_t PktLength = 0, IpVersion = 0;
    PyObject* MemPool = NULL;
    bool L2IsEther = false;
    
    if (!PyArg_ParseTupleAndKeywords(Args, Kwds, "I|OsssH", KwdList, &Self->Length, &MemPool, &L2, &L3, &L4, &PktLength))
        return -1;
    
    PktLength = std::max(PktLength, (uint16_t)8);
    
    Self->Pkts = new struct rte_mbuf* [Self->Length];
    
    if (unlikely(Self->Pkts == NULL)) {
        PyErr_SetString(PyExc_MemoryError, "Cannot alloc space for packet array.");
        return -1;
    }
    
    if (MemPool != NULL) {
        if (!PyObject_TypeCheck(MemPool, &MemPool_Type)) {
            PyErr_SetString(PyExc_TypeError, "Argument MemPool is not MemPool type.");
            return -1;
        }
        if (rte_pktmbuf_alloc_bulk(((MemPool_Object*)MemPool)->MemPoolPtr, Self->Pkts, Self->Length)) {
            PyErr_SetString(PyExc_MemoryError, "Cannot alloc Mbuf from MemPool."
                                               "Please check if mempool has enough descriptors.");
            return -1;
        }
        Self->AvilLength = Self->Length;
    }
    else {
        Self->AvilLength = 0;
        std::memset(Self->Pkts, 0, sizeof(struct rte_mbuf*) * Self->Length);
        return 0;
    }
    
    struct rte_mbuf* Data = Self->Pkts[0];
    
    Data->packet_type = 0;
    Data->next = NULL;
    Data->nb_segs = 1;
    Data->refcnt = 1;
    
    if (!std::strcmp(L2, "Ethernet")) {
        Data->packet_type |= RTE_PTYPE_L2_ETHER;
        Data->l2_len = sizeof(struct rte_ether_hdr);
        L2IsEther = true;
    } else if (!std::strcmp(L3, "Ethernet-ARP")){
        Data->packet_type |= RTE_PTYPE_L2_ETHER_ARP;
        Data->l2_len = sizeof(struct rte_ether_hdr) + sizeof(struct rte_arp_hdr);
        InitEthHeader((struct rte_ether_hdr*)L2HeaderPtr, RTE_ETHER_TYPE_ARP, false, 0);
        InitArpHeader((struct rte_arp_hdr*)((uint8_t *)L2HeaderPtr + sizeof(struct rte_ether_hdr)));
        return 0;
    } else {
        PyErr_SetString(PyExc_ValueError, "Unknown L2 argument.");
        return -1;
    }
    
    if (L3 != NULL) {
        if (!std::strcmp(L3, "IPv4")) {
            Data->packet_type |= RTE_PTYPE_L3_IPV4;
            Data->ol_flags = RTE_MBUF_F_TX_IPV4;
            Data->l3_len = sizeof(struct rte_ipv4_hdr);
            IpVersion = 4;
            if (L2IsEther)
                InitEthHeader((struct rte_ether_hdr*)L2HeaderPtr, RTE_ETHER_TYPE_IPV4, false, 0);
        } else if (!std::strcmp(L3, "IPv6")){
            Data->packet_type |= RTE_PTYPE_L3_IPV6;
            Data->ol_flags = RTE_MBUF_F_TX_IPV6;
            Data->l3_len = sizeof(struct rte_ipv6_hdr);
            IpVersion = 6;
            if (L2IsEther)
                InitEthHeader((struct rte_ether_hdr*)L2HeaderPtr, RTE_ETHER_TYPE_IPV6, false, 0);
        } else {
            PyErr_SetString(PyExc_ValueError, "Unknown L3 argument.");
            return -1;
        }
    } 
    
    if ((L3 != NULL) && (L4 != NULL)) {
        if (!std::strcmp(L4, "UDP")) {
            Data->packet_type |= RTE_PTYPE_L4_UDP;
            Data->l4_len = sizeof(struct rte_udp_hdr);
            PktLength += Data->l4_len;
            InitUDPHeader((struct rte_udp_hdr*)L4HeaderPtr, PktLength);
            if (IpVersion == 4) {
                PktLength += Data->l3_len;
                InitIPv4Header((struct rte_ipv4_hdr*)L3HeaderPtr, PktLength, IPPROTO_UDP);
            }
            else if (IpVersion == 6) {
                InitIPv6Header((struct rte_ipv6_hdr*)L3HeaderPtr, PktLength, IPPROTO_UDP);
                PktLength += Data->l3_len;
            }
            else {
                PyErr_SetString(PyExc_ValueError, "L3 protocol not specified.");
                return -1;
            }
        } else if (!std::strcmp(L4, "TCP")){
            Data->packet_type |= RTE_PTYPE_L4_TCP;
            Data->l4_len = sizeof(struct rte_tcp_hdr);
            PktLength += Data->l4_len;
            InitTCPHeader((struct rte_tcp_hdr*)L4HeaderPtr, PktLength);
            if (IpVersion == 4) {
                PktLength += Data->l3_len;
                InitIPv4Header((struct rte_ipv4_hdr*)L3HeaderPtr, PktLength, IPPROTO_TCP);
            }
            else if (IpVersion == 6) {
                InitIPv6Header((struct rte_ipv6_hdr*)L3HeaderPtr, PktLength, IPPROTO_TCP);
                PktLength += Data->l3_len;
            }
            else {
                PyErr_SetString(PyExc_ValueError, "L3 protocol not specified.");
                return -1;
            }
        } else {
            PyErr_SetString(PyExc_ValueError, "Unknown L4 argument.");
            return -1;
        }
    }
    
    PktLength += Data->l2_len;
    if (unlikely((int)(PktLength - Data->l2_len) < 46)) {
        printf("Packet\'s Ethernet frame load length less than 46 bytes, padding automatically.\n");
        PktLength += (uint16_t)(46 - (int)(PktLength - Data->l2_len));
    }
    if (unlikely(Data->l2_len + (uint16_t)1500 < PktLength)) {
        printf("Packet\'s Ethernet frame load length more than 1500 bytes, cut off automatically.\n");
        uint16_t Offset = PktLength - (Data->l2_len + (uint16_t)1500);
        PktLength = Data->l2_len + (uint16_t)1500;
         
        if (!std::strcmp(L4, "UDP")) {
            uint16_t Temp = rte_be_to_cpu_16(((struct rte_udp_hdr*)L4HeaderPtr)->dgram_len);
            Temp -= Offset;
            ((struct rte_udp_hdr*)L4HeaderPtr)->dgram_len = rte_cpu_to_be_16(Temp);
        }
        if (IpVersion == 4) {
            uint16_t Temp = rte_be_to_cpu_16(((struct rte_ipv4_hdr*)L3HeaderPtr)->total_length);
            Temp -= Offset;
            ((struct rte_ipv4_hdr*)L3HeaderPtr)->total_length = rte_cpu_to_be_16(Temp);
        }
        else if (IpVersion == 6) {
            uint16_t Temp = rte_be_to_cpu_16(((struct rte_ipv6_hdr*)L3HeaderPtr)->payload_len);
            Temp -= Offset;
            ((struct rte_ipv6_hdr*)L3HeaderPtr)->payload_len = rte_cpu_to_be_16(Temp);
        }
    }
    Data->data_len = Data->pkt_len = PktLength;
    
    void* DataSeg = (void*)rte_pktmbuf_mtod(Self->Pkts[0], void*);
    for (uint32_t Index = 1; Index < Self->Length; ++Index) {
        Self->Pkts[Index]->packet_type = 0;
        Self->Pkts[Index]->next = NULL;
        Self->Pkts[Index]->nb_segs = 1;
        Self->Pkts[Index]->refcnt = 1;
        Self->Pkts[Index]->packet_type = Self->Pkts[0]->packet_type;
        Self->Pkts[Index]->ol_flags = Self->Pkts[0]->ol_flags;
        Self->Pkts[Index]->l2_len = Self->Pkts[0]->l2_len;
        Self->Pkts[Index]->l3_len = Self->Pkts[0]->l3_len;
        Self->Pkts[Index]->l4_len = Self->Pkts[0]->l4_len;
        Self->Pkts[Index]->data_len = Self->Pkts[0]->data_len;
        Self->Pkts[Index]->pkt_len = Self->Pkts[0]->pkt_len;
        rte_memcpy(rte_pktmbuf_mtod(Self->Pkts[Index], void*), DataSeg, PktLength);
    }
    
    return 0;
}

PyObject* PacketArray_Fill(PacketArray_Object* Self, PyObject *Args, PyObject *Kwds) {
    //TODO
    Py_RETURN_NONE;
}

Py_ssize_t PacketArray_Length(PacketArray_Object* PktArray) {
    return (Py_ssize_t)PktArray->Length;
}

PyObject* PacketArray_Concat(PacketArray_Object *Object1, PacketArray_Object* Object2) {
    PyErr_SetString(PyExc_RuntimeError, "Packet contact should not be called due to mempool access.\n"
                                        "See README for more infomation.");
    return NULL;
}

PyObject* PacketArray_Repeat(PacketArray_Object *Object, Py_ssize_t Count) {
    PyErr_SetString(PyExc_RuntimeError, "Packet repeat should not be called due to mempool access.\n"
                                        "See README for more infomation.");
    return NULL;
}

PyObject* PacketArray_InPlaceConcat(PacketArray_Object *Object1, PacketArray_Object* Object2) {
    PyErr_SetString(PyExc_RuntimeError, "Packet contact should not be called due to mempool access.\n"
                                        "See README for more infomation.");
    return NULL;
}

PyObject* PacketArray_InPlaceRepeat(PacketArray_Object *Object, Py_ssize_t Count) {
    PyErr_SetString(PyExc_RuntimeError, "Packet repeat should not be called due to mempool access.\n"
                                        "See README for more infomation.");
    return NULL;
}

PyObject* PacketArray_GetItem(PacketArray_Object* PktArray, Py_ssize_t Index) {
    if (unlikely((Index >= PktArray->Length) || (Index < 0))) {
        PyErr_SetString(PyExc_IndexError, "Index invalid when access to packet array.");
        return NULL;
    }
    Packet_Object* Ptr = (Packet_Object*)Packet_Type.tp_alloc(&Packet_Type, 0);
    Ptr->Data = PktArray->Pkts[Index];
    return (PyObject*)Ptr;
}

int PacketArray_Type_Init() {
    PacketArray_Type.tp_name = "Pktgen_DPDK.PacketArray";
    PacketArray_Type.tp_doc = PyDoc_STR("Packet Array objects.");
    PacketArray_Type.tp_basicsize = sizeof(PacketArray_Object);
    PacketArray_Type.tp_itemsize = 0;
    PacketArray_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    PacketArray_Type.tp_new = PacketArray_New;
    PacketArray_Type.tp_init = (initproc)PacketArray_Init;
    PacketArray_Type.tp_dealloc = (destructor)PacketArray_Dealloc;
    PacketArray_Type.tp_members = PacketArray_Members;
    PacketArray_Type.tp_methods = PacketArray_Methods;
    PacketArray_Type.tp_getset = PacketArray_GetSetters;
    PacketArray_Type.tp_as_sequence = &PacketArray_SeqMethods;
    return PyType_Ready(&PacketArray_Type);
}
