#include "Device.h"

extern std::atomic<int> __Builtin_ArrayCounter;

std::atomic<int> __Builtin_DeviceCounter(0); 

extern "C" {
    static PyMemberDef Device_Members[] = {
        {NULL}
    };
    
    static PyMethodDef Device_Methods[] = { {
            .ml_name = "Config",
            .ml_meth = (PyCFunction)Device_Config,
            .ml_flags = METH_NOARGS,
            .ml_doc = "Config a device(ethernet port)."
        },{
            .ml_name = "GetMAC",
            .ml_meth = (PyCFunction)Device_GetMAC,
            .ml_flags = METH_NOARGS,
            .ml_doc = "Get the MAC address of a device(ethernet port)."
        },{
            .ml_name = "Receive",
            .ml_meth = (PyCFunction)Device_Receive,
            .ml_flags = METH_VARARGS,
            .ml_doc = "Receive a number of packets."
        },{
            .ml_name = "Send",
            .ml_meth = (PyCFunction)Device_Send,
            .ml_flags = METH_VARARGS,
            .ml_doc = "Send a number of packets immediately."
        },{
            .ml_name = "SendWithDelay",
            .ml_meth = (PyCFunction)Device_SendWithDelay,
            .ml_flags = METH_VARARGS,
            .ml_doc = "Send a number of packets with time delay specified."
        },{
            .ml_name = "SendWithRate",
            .ml_meth = (PyCFunction)Device_SendWithRate,
            .ml_flags = METH_VARARGS,
            .ml_doc = "Send a number of packets with rate specified."
        },{
            .ml_name = "SendWithPossionDelay",
            .ml_meth = (PyCFunction)Device_SendWithPossionDelay,
            .ml_flags = METH_VARARGS,
            .ml_doc = "Send a number of packets with delay satisfing negative exponential distribution."
        },
        {NULL}
    };
    
    static PyGetSetDef Device_GetSetters[] = {
        {NULL}
    };
    
    PyTypeObject Device_Type = {
        PyVarObject_HEAD_INIT(NULL, 0)
    };
} 

void Device_Dealloc(Device_Object *Self) {
    if (Self->Start) {
        printf("Closing device %d...\n", Self->Info.Port);
       	int Res = rte_eth_dev_stop(Self->Info.Port);
       	if (Res != 0) printf("Function rte_eth_dev_stop: error=%d, device=%d\n", Res, Self->Info.Port);
       	rte_eth_dev_close(Self->Info.Port);
       	printf("Device %d has been closed.\n", Self->Info.Port);
    }
    if (--__Builtin_DeviceCounter == 0) {
        if (__Builtin_ArrayCounter == 0)
            rte_eal_cleanup();
    }
   	Py_TYPE(Self)->tp_free((PyObject*)Self);
}

PyObject* Device_New(PyTypeObject *Type, PyObject *Args, PyObject *Kwds) {
    Device_Object* Self;
    
    __Builtin_DeviceCounter.fetch_add(1);
    Self = (Device_Object*)Type->tp_alloc(Type, 0);
    std::memset(&Self->Info, 0, sizeof(struct EthConfigInfo));
    return (PyObject*) Self;
}

int Device_Init(Device_Object* Self, PyObject *Args, PyObject *Kwds) {
    static char* KwdList[] = {"Port", "RxQueues", "TxQueues", "RxDescs", "TxDescs", "Promiscuous", NULL};

    if (!PyArg_ParseTupleAndKeywords(Args, Kwds, "IHH|HHp", KwdList,
                                     &(Self->Info.Port), &(Self->Info.RxQueues), &(Self->Info.TxQueues),
	                                 &(Self->Info.RxDescs), &(Self->Info.TxDescs), &(Self->Info.Promiscuous)))
        return -1;
    return 0;
}

PyObject* Device_Config(Device_Object* Self, PyObject* Py_UNUSED(ignored)) {
    if (ConfigDevice(&Self->Info, Self->MACAddress) < 0) {
        PyErr_SetString(PyExc_RuntimeError, "Config error. Please check device parameters."); 
        return NULL;
    }
    Self->Start = true;
    Py_RETURN_NONE;
}

PyObject* Device_GetMAC(Device_Object* Self, PyObject* Py_UNUSED(ignored)) {
    return PyUnicode_FromString(Self->MACAddress);
}

int Device_Type_Init() {
    Device_Type.tp_name = "Pktgen_DPDK.Device";
    Device_Type.tp_doc = PyDoc_STR("Device(ethernet port) objects.");
    Device_Type.tp_basicsize = sizeof(Device_Object);
    Device_Type.tp_itemsize = 0;
    Device_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    Device_Type.tp_new = Device_New;
    Device_Type.tp_init = (initproc)Device_Init;
    Device_Type.tp_dealloc = (destructor)Device_Dealloc;
    Device_Type.tp_members = Device_Members;
    Device_Type.tp_methods = Device_Methods;
    Device_Type.tp_getset = Device_GetSetters;
    return PyType_Ready(&Device_Type);
}
