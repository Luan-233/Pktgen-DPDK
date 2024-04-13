import Pktgen_DPDK as Gen
import random

port = Gen.Device(0, 1, 1)
port.Config()

mp = Gen.MemPool(16)

pkts = Gen.PacketArray(16, MemPool = mp, L2 = "Ethernet", L3 = "IPv4", L4 = "UDP")

delay = []

for i in range(0, pkts.Length):
    pkts[i].Ethernet = { "Src-MAC" : port.GetMAC(), "Dst-MAC" : "04-33-C2-71-65-B5", "Ether-Type" : 0x0800 }
    pkts[i].IPv4 = { "Src-IP" : "192.168.233." + str(random.randint(2, 254)), "Dst-IP" : "192.168.31.15" }
    pkts[i].UDP = { "Src-Port" : 12345, "Dst-Port" : random.randint(12345, 23456)}
    pkts[i].CalcL3CkSum()
    pkts[i].CalcL4CkSum()
    delay.append((i + 1) * 10);

port.SendWithDelay(0, pkts, delay)

for i in range(0, pkts.Length):
    print(pkts[i].IPv4)
    print(pkts[i].UDP)