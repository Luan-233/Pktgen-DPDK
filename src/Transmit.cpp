#include "Device.h"

extern PyTypeObject PacketArray_Type;

PyObject* Device_Receive(Device_Object* Self, PyObject *Args) {
    uint16_t QueueId = 0, Count = 0;
    PacketArray_Object* PktArray = NULL;
    
    if (!PyArg_ParseTuple(Args, "HO", &QueueId, &PktArray))
        return NULL;
    
    rte_pktmbuf_free_bulk(PktArray->Pkts, PktArray->Length);
    std::memset(PktArray->Pkts, 0, sizeof(struct rte_mbuf*) * PktArray->Length);
    
    Count = rte_eth_rx_burst(Self->Info.Port, QueueId, PktArray->Pkts, PktArray->Length);
    return PyLong_FromUnsignedLong((unsigned long)Count);
}

PyObject* Device_Send(Device_Object* Self, PyObject *Args) {
    uint16_t QueueId = 0, Count = 0, Length = 0;
    PyObject* PktArray = NULL;
    
    if (!PyArg_ParseTuple(Args, "HO", &QueueId, &PktArray))
        return NULL;
    
    if (!PyObject_TypeCheck(PktArray, &PacketArray_Type)) {
        PyErr_SetString(PyExc_TypeError, "Packet array argument is not expected type \"PacketAray\".");
        return NULL;
    }
    
    Length = ((PacketArray_Object*)PktArray)->Length;
    Count = rte_eth_tx_burst(Self->Info.Port, QueueId, ((PacketArray_Object*)PktArray)->Pkts, Length);
    return PyLong_FromUnsignedLong((unsigned long)Count);
}

PyObject* Device_SendWithDelay(Device_Object* Self, PyObject *Args) {
    uint64_t Frequency = rte_get_tsc_hz();
    uint64_t NextSend = rte_get_tsc_cycles();
    uint16_t QueueId = 0, Count = 0;
    double Delay = 0;
    PyObject *PktArray = NULL, *DelayArray = NULL, *DelayObj = NULL;
    
    if (!PyArg_ParseTuple(Args, "HOO", &QueueId, &PktArray, &DelayArray))
        return NULL;
    
    if (!PyObject_TypeCheck(PktArray, &PacketArray_Type)) {
        PyErr_SetString(PyExc_TypeError, "Packet array argument is not expected type \"PacketAray\".");
        return NULL;
    }
    
    for (uint32_t Index = 0; Index < ((PacketArray_Object*)PktArray)->Length; ++Index) {
        DelayObj = PyList_GetItem(DelayArray, (Py_ssize_t)Index);
        if (PyLong_Check(DelayObj))
            Delay = PyLong_AsLong(DelayObj);
        else if (PyFloat_Check(DelayObj))
            Delay = PyFloat_AsDouble(DelayObj);
        else {
            PyErr_SetString(PyExc_TypeError, "Argument Delay array has objects which are not number type.");
            return NULL;
        }
        NextSend += (uint64_t)(Delay / 1000.0 * (double)Frequency);
        while (rte_get_tsc_cycles() < NextSend);
        while (!rte_eth_tx_burst(Self->Info.Port, QueueId, &((PacketArray_Object*)PktArray)->Pkts[Index], 1));
        ++Count;
    }
    return PyLong_FromUnsignedLong((unsigned long)Count);
}

PyObject* Device_SendWithRate(Device_Object* Self, PyObject *Args) {
    uint64_t Frequency = rte_get_tsc_hz();
    uint64_t NextSend = rte_get_tsc_cycles();
    uint16_t QueueId = 0, Count = 0;
    uint64_t Bps = 0;
    PyObject *PktArray = NULL, *RateObj = NULL;
    
    if (!PyArg_ParseTuple(Args, "HOO", &QueueId, &PktArray, &RateObj))
        return NULL;
    
    if (!PyObject_TypeCheck(PktArray, &PacketArray_Type)) {
        PyErr_SetString(PyExc_TypeError, "Packet array argument is not expected type \"PacketAray\".");
        return NULL;
    }
        
    if (PyLong_Check(RateObj))
        Bps = (uint64_t)((double)PyLong_AsLong(RateObj) * 1000000.0);
    else if (PyFloat_Check(RateObj))
        Bps = (uint64_t)((double)PyFloat_AsDouble(RateObj) * 1000000.0);
    else {
        PyErr_SetString(PyExc_TypeError, "Argument Rate is not a number type.");
        return NULL;
    }
    
    for (uint32_t Index = 0; Index < ((PacketArray_Object*)PktArray)->Length; ++Index) {
        NextSend += ((PacketArray_Object*)PktArray)->Pkts[Index]->pkt_len * 8 / Bps * Frequency;
        while (rte_get_tsc_cycles() < NextSend);
        while (!rte_eth_tx_burst(Self->Info.Port, QueueId, &((PacketArray_Object*)PktArray)->Pkts[Index], 1));
        ++Count;
    }
    return PyLong_FromUnsignedLong((unsigned long)Count);
}

PyObject* Device_SendWithPossionDelay(Device_Object* Self, PyObject *Args) {
    uint64_t Frequency = rte_get_tsc_hz();
    uint64_t NextSend = rte_get_tsc_cycles();
    uint16_t QueueId = 0, Count = 0;
    double Delay = 0, Lambda = 0;
    PyObject *PktArray = NULL, *LambdaObj = NULL;
    
    if (!PyArg_ParseTuple(Args, "HOO", &QueueId, &PktArray, &LambdaObj))
        return NULL;
    
    if (!PyObject_TypeCheck(PktArray, &PacketArray_Type)) {
        PyErr_SetString(PyExc_TypeError, "Packet array argument is not expected type \"PacketAray\".");
        return NULL;
    }
    
    if (PyLong_Check(LambdaObj))
        Lambda = (double)PyLong_AsLong(LambdaObj);
    else if (PyFloat_Check(LambdaObj))
        Lambda = (double)PyFloat_AsDouble(LambdaObj);
    else {
        PyErr_SetString(PyExc_TypeError, "Argument Lambda is not a number type.");
        return NULL;
    }
    
    std::mt19937 Generator((uint32_t)std::time(NULL));
    std::exponential_distribution<double> Distribution(Lambda);
    
    for (uint32_t Index = 0; Index < ((PacketArray_Object*)PktArray)->Length; ++Index) {
        Delay = Distribution(Generator);
        NextSend += (uint64_t)(Delay / 1000.0 * (double)Frequency);
        while (rte_get_tsc_cycles() < NextSend);
        while (!rte_eth_tx_burst(Self->Info.Port, QueueId, &((PacketArray_Object*)PktArray)->Pkts[Index], 1));
        ++Count;
    }
    return PyLong_FromUnsignedLong((unsigned long)Count);
}
