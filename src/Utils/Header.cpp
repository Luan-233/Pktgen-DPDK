#include "Header.h"

void InitEthHeader(struct rte_ether_hdr *EthHeader, uint16_t EthType, bool VlanEnabled, uint16_t VlanId) {
	if (VlanEnabled) {
		struct rte_vlan_hdr *VlanHeader = (struct rte_vlan_hdr *)((uint8_t *)EthHeader + sizeof(struct rte_ether_hdr));
		EthHeader->ether_type = rte_cpu_to_be_16(RTE_ETHER_TYPE_VLAN);
		VlanHeader->eth_proto =  rte_cpu_to_be_16(EthType);
		VlanHeader->vlan_tci = VlanId;
	} 
    else EthHeader->ether_type = rte_cpu_to_be_16(EthType);
}

void InitArpHeader(struct rte_arp_hdr *ARPHeader) {
    ARPHeader->arp_hardware = rte_cpu_to_be_16(RTE_ARP_HRD_ETHER);
	ARPHeader->arp_protocol = rte_cpu_to_be_16(RTE_ETHER_TYPE_IPV4);
	ARPHeader->arp_hlen = RTE_ETHER_ADDR_LEN;
	ARPHeader->arp_plen = sizeof(uint32_t);
}

void InitUDPHeader(struct rte_udp_hdr *UDPHeader, uint16_t DataLen) {
	UDPHeader->dgram_len = rte_cpu_to_be_16(DataLen);
	UDPHeader->dgram_cksum = 0;
}

void InitTCPHeader(struct rte_tcp_hdr *TCPHeader, uint16_t DataLen) {
    std::memset(TCPHeader, 0, sizeof(struct rte_tcp_hdr));
	TCPHeader->data_off = (sizeof(struct rte_tcp_hdr) << 2) & 0xF0;
}

void InitIPv4Header(struct rte_ipv4_hdr *IPv4Header, uint16_t DataLen, uint8_t Proto) {
	IPv4Header->version_ihl = RTE_IPV4_VHL_DEF;
	IPv4Header->type_of_service = 0;
	IPv4Header->fragment_offset = 0;
	IPv4Header->time_to_live = IP_DEFTTL;
	IPv4Header->next_proto_id = Proto;
	IPv4Header->packet_id = 0;
	IPv4Header->total_length = rte_cpu_to_be_16(DataLen);
}

void InitIPv6Header(struct rte_ipv6_hdr *IPv6Header, uint16_t DataLen, uint8_t Proto) {
    IPv6Header->vtc_flow = rte_cpu_to_be_32(0x60000000);
	IPv6Header->payload_len = rte_cpu_to_be_16(DataLen);
	IPv6Header->proto = Proto;
	IPv6Header->hop_limits = IP_DEFTTL;
}
