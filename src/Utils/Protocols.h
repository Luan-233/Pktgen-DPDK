#include <rte_ip.h>
#include <rte_tcp.h>

#include <cstdio>
#include <cstring>
#include <netinet/in.h>

#ifndef IPPROTO_ICMP
    #define IPPROTO_ICMP 1
#endif

#ifndef IPPROTO_IGMP
    #define IPPROTO_IGMP 2
#endif

#ifndef IPPROTO_OSPF
    #define IPPROTO_OSPF 89
#endif

#ifndef IPPROTO_SCTP
    #define IPPROTO_SCTP 132
#endif

extern "C" {
    const char* GetIPProto(uint8_t ProtoId);
    int GetTCPFlags(uint8_t TCPFlags, char* Str);
}
