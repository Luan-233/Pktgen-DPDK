#include "Packet.h"

extern "C" {
    static PyMemberDef Packet_Members[] = {
        {NULL}
    };
    
    static PyMethodDef Packet_Methods[] = { {
            .ml_name = "Show",
            .ml_meth = (PyCFunction)Packet_Show,
            .ml_flags = METH_NOARGS,
            .ml_doc = "Print all information of a packet."
        },{
            .ml_name = "TimeStamp",
            .ml_meth = (PyCFunction)Packet_TimeStamp,
            .ml_flags = METH_NOARGS,
            .ml_doc = "Make time stamp for a packet and return it."
        },{
            .ml_name = "CalcL3CkSum",
            .ml_meth = (PyCFunction)Packet_CalcL3CkSum,
            .ml_flags = METH_NOARGS,
            .ml_doc = "Calculate L3 layer check sum and fill it."
        },{
            .ml_name = "CalcL4CkSum",
            .ml_meth = (PyCFunction)Packet_CalcL4CkSum,
            .ml_flags = METH_NOARGS,
            .ml_doc = "Calculate L4 layer check sum and fill it."
        },
        {NULL}
    };
    
    static PyGetSetDef Packet_GetSetters[] = { {
            .name = "Ethernet",
            .get = (getter)Packet_Get_Ethernet,
            .set = (setter)Packet_Set_Ethernet,
            .doc = "Information of ethernet header.",
            .closure = NULL
        },{
            .name = "IPv4",
            .get = (getter)Packet_Get_IPv4,
            .set = (setter)Packet_Set_IPv4,
            .doc = "Information of Ipv4 header.",
            .closure = NULL
        },{
            .name = "ARP",
            .get = (getter)Packet_Get_ARP,
            .set = (setter)Packet_Set_ARP,
            .doc = "Information of ARP header.",
            .closure = NULL
        },{
            .name = "IPv6",
            .get = (getter)Packet_Get_IPv6,
            .set = (setter)Packet_Set_IPv6,
            .doc = "Information of Ipv5 header.",
            .closure = NULL
        },{
            .name = "TCP",
            .get = (getter)Packet_Get_TCP,
            .set = (setter)Packet_Set_TCP,
            .doc = "Information of TCP header.",
            .closure = NULL
        },{
            .name = "UDP",
            .get = (getter)Packet_Get_UDP,
            .set = (setter)Packet_Set_UDP,
            .doc = "Information of UDP header.",
            .closure = NULL
        },
        {NULL}
    };
    
    PyTypeObject Packet_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
    };
}

static inline uint64_t ReadTSC() {
    uint32_t Low, High;
    asm volatile("rdtsc" : "=a"(Low), "=d"(High));
    return ((uint64_t)High << 32) | ((uint64_t)Low);
}

void Packet_Dealloc(Packet_Object *Self) {
    Py_TYPE(Self)->tp_free((PyObject*)Self);
}

PyObject* Packet_New(PyTypeObject *Type, PyObject *Args, PyObject *Kwds) {
    Packet_Object* Self;
    Self = (Packet_Object*)Type->tp_alloc(Type, 0);
    Self->Data = NULL;
    return (PyObject*) Self;
}

int Packet_Init(Packet_Object* Self, PyObject *Args, PyObject *Kwds) {
    PyErr_SetString(PyExc_TypeError, "Packet initalizer should not be called due to mempool access.\n"
                                     "See README for more infomation.");
    return 0;
}

PyObject* Packet_Show(Packet_Object* Self, PyObject* Py_UNUSED(ignored)) {
    struct rte_mbuf* Data = Self->Data;
    printf("Layer 2: ");
    switch (Data->packet_type & RTE_PTYPE_L2_MASK) {
        case RTE_PTYPE_L2_ETHER:        printf("Ethernet.\n");          break;
        case RTE_PTYPE_L2_ETHER_ARP:    printf("Ethernet with ARP.\n"); Py_RETURN_NONE;
        default:
            PyErr_SetString(PyExc_ValueError, "Unknown layer 2 tag. Please check if L4 is set.");
            return NULL;
    }
    printf("Layer 3: ");
    switch (Data->packet_type & RTE_PTYPE_L3_MASK) {
        case RTE_PTYPE_L3_IPV4:         printf("IPv4.\n");              break;
        case RTE_PTYPE_L3_IPV6:         printf("IPv6.\n");              break;
        default:
            PyErr_SetString(PyExc_ValueError, "Unknown layer 3 tag. Please check if L3 is set.");
    }
    switch (Data->packet_type & RTE_PTYPE_L4_MASK) {
        case RTE_PTYPE_L4_TCP:          printf("TCP.\n");               break;
        case RTE_PTYPE_L4_UDP:          printf("UDP.\n");               break;
        case RTE_PTYPE_L4_SCTP:         printf("SCTP(Unsupport).\n");   break;
        case RTE_PTYPE_L4_ICMP:         printf("ICMP(Unsupport).\n");   break;
        case RTE_PTYPE_L4_IGMP:         printf("IGMP(Unsupport).\n");   break;
        default:
            PyErr_SetString(PyExc_ValueError, "Unknown layer 4 tag. Please check if L4 is set.");
    }
    printf("For detalied infomation, please access to each layer\'s filed.\n");
    Py_RETURN_NONE;
}

PyObject* Packet_TimeStamp(Packet_Object* Self, PyObject* Py_UNUSED(ignored)) {
    struct rte_mbuf* Data = Self->Data;
    *(uint64_t*)DataPtr = ReadTSC();
    return PyLong_FromUnsignedLong(*(uint64_t*)DataPtr);
}

static void CalcIpv4CheckSum(struct rte_ipv4_hdr* Ipv4Header) {
    uint32_t CkSum = 0;
    unaligned_uint16_t *Ptr16 = (unaligned_uint16_t *)Ipv4Header;
	CkSum += Ptr16[0]; CkSum += Ptr16[1];
	CkSum += Ptr16[2]; CkSum += Ptr16[3];
	CkSum += Ptr16[4];
	CkSum += Ptr16[6]; CkSum += Ptr16[7];
	CkSum += Ptr16[8]; CkSum += Ptr16[9];
	CkSum = ((CkSum & 0xFFFF0000) >> 16) + (CkSum & 0x0000FFFF);
	CkSum = (~(CkSum & 0x0000FFFF));
	if (CkSum == 0) CkSum = 0xFFFF;
	Ipv4Header->hdr_checksum = (uint16_t)CkSum;
}

PyObject* Packet_CalcL3CkSum(Packet_Object* Self, PyObject* Py_UNUSED(ignored)) {
    struct rte_mbuf* Data = Self->Data;
    switch (Data->packet_type & RTE_PTYPE_L3_MASK) {
        case RTE_PTYPE_L3_IPV4:
            CalcIpv4CheckSum((struct rte_ipv4_hdr*)L3HeaderPtr);
            break;
        case RTE_PTYPE_L3_IPV6:
            //Ipv6 has no check sum 
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Unknown layer 3 tag. Please check if L3 is set.");
            return NULL;
    }
    Py_RETURN_NONE;
}

PyObject* Packet_CalcL4CkSum(Packet_Object* Self, PyObject* Py_UNUSED(ignored)) {
    unaligned_uint16_t* CkSum = NULL;
    struct rte_mbuf* Data = Self->Data;
    switch (Data->packet_type & RTE_PTYPE_L4_MASK) {
        case RTE_PTYPE_L4_TCP:
            CkSum = &((struct rte_tcp_hdr*)L4HeaderPtr)->cksum;
            break;
        case RTE_PTYPE_L4_UDP:
            CkSum = &((struct rte_udp_hdr*)L4HeaderPtr)->dgram_cksum;
            break;
        default:
            PyErr_SetString(PyExc_ValueError, "Unknown layer 4 tag. Please check if L4 is set.");
            return NULL;
    }
    (*CkSum) = (rte_be16_t)0;
    switch (Data->packet_type & RTE_PTYPE_L3_MASK) {
        case RTE_PTYPE_L3_IPV4:
            (*CkSum) = rte_ipv4_udptcp_cksum((struct rte_ipv4_hdr*)L3HeaderPtr, L4HeaderPtr);
            break;
        case RTE_PTYPE_L3_IPV6:
            (*CkSum) = rte_ipv6_udptcp_cksum((struct rte_ipv6_hdr*)L3HeaderPtr, L4HeaderPtr);
            break;
    }
    Py_RETURN_NONE;
}

int Packet_Type_Init() {
    Packet_Type.tp_name = "Pktgen_DPDK.Packet";
    Packet_Type.tp_doc = PyDoc_STR("Packet objects.");
    Packet_Type.tp_basicsize = sizeof(Packet_Object);
    Packet_Type.tp_itemsize = 0;
    Packet_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    Packet_Type.tp_new = Packet_New;
    Packet_Type.tp_init = (initproc)Packet_Init;
    Packet_Type.tp_dealloc = (destructor)Packet_Dealloc;
    Packet_Type.tp_members = Packet_Members;
    Packet_Type.tp_methods = Packet_Methods;
    Packet_Type.tp_getset = Packet_GetSetters;
    return PyType_Ready(&Packet_Type);
}
