#include "Protocols.h"

static const char* IPProto[1 << 8];
static const char* TCPFlag[1 << 3];

static uint32_t Log2(uint32_t X) {
    uint32_t Res = 0;
    while (X > 1) ++Res, X >>= 1;
    return Res;
}

__attribute__((constructor)) void ProtoInit() {
    std::memset(IPProto, 0, sizeof(const char*) * (1 << 8));
    IPProto[IPPROTO_ICMP] = "ICMP";
    IPProto[IPPROTO_IGMP] = "IGMP";
    IPProto[IPPROTO_TCP] = "TCP";
    IPProto[IPPROTO_UDP] = "UDP";
    IPProto[IPPROTO_OSPF] = "OSPF";
    IPProto[IPPROTO_SCTP] = "SCTP";
    
    std::memset(TCPFlag, 0, sizeof(const char*) * (1 << 3));
    TCPFlag[Log2(RTE_TCP_CWR_FLAG)] = "CWR";
    TCPFlag[Log2(RTE_TCP_ECE_FLAG)] = "ECE";
    TCPFlag[Log2(RTE_TCP_URG_FLAG)] = "URG";
    TCPFlag[Log2(RTE_TCP_ACK_FLAG)] = "ACK";
    TCPFlag[Log2(RTE_TCP_PSH_FLAG)] = "PSH";
    TCPFlag[Log2(RTE_TCP_RST_FLAG)] = "RST";
    TCPFlag[Log2(RTE_TCP_SYN_FLAG)] = "SYN";
    TCPFlag[Log2(RTE_TCP_FIN_FLAG)] = "FIN";
}

const char* GetIPProto(uint8_t ProtoId) {
    return IPProto[ProtoId];
}

int GetTCPFlags(uint8_t TCPFlags, char* Str) {
    uint32_t Count = 0;
    for (int Bit = 7; Bit >= 0; --Bit) {
        if (TCPFlags | (1 << Bit)) {
            if (!Count) Count += sprintf(&Str[Count], "%s", TCPFlag[Bit]);
            else Count += sprintf(&Str[Count], "-%s", TCPFlag[Bit]);
        }
    }
    Str[Count] = '\0';
    return Count;
}
