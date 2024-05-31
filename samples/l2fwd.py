import Pktgen_DPDK as Gen

port1 = Gen.Device(0, 1, 1)
port1.Config()

port2 = Gen.Device(1, 1, 1)
port2.Config()

pkts = Gen.PacketArray(16)

while True:
    port1.Receive(0, pkts)
    port2.Send(0, pkts)
    port2.Receive(0, pkts)
    port1.Send(0, pkts)