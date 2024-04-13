#pragma once

#include <rte_ip.h>
#include <rte_arp.h>
#include <rte_tcp.h>
#include <rte_udp.h>
#include <rte_ether.h>
#include <rte_common.h>
#include <rte_config.h>

#include <cstring>
#include <string.h> 
#include <stdint.h>
#include <netinet/in.h>

#ifndef IP_DEFTTL
    #ifdef IPDEFTTL
        #define IP_DEFTTL IPDEFTTL
    #else
        #define IP_DEFTTL 32
    #endif
#endif

extern "C" {
    void InitEthHeader(struct rte_ether_hdr* EthHeader, uint16_t EthType, bool VlanEnabled, uint16_t VlanId);
    void InitArpHeader(struct rte_arp_hdr* ARPHeader);
    void InitUDPHeader(struct rte_udp_hdr* UDPHeader, uint16_t DataLen);
    void InitTCPHeader(struct rte_tcp_hdr* TCPHeader, uint16_t DataLen);
    void InitIPv4Header(struct rte_ipv4_hdr* IPv4Header, uint16_t DataLen, uint8_t Proto);
    void InitIPv6Header(struct rte_ipv6_hdr* IPv6Header, uint16_t DataLen, uint8_t Proto);
}
