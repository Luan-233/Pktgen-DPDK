#include "Config.h"

int ConfigDevice(struct EthConfigInfo* Info, char* MACAddress) {
    struct rte_eth_rxconf RxConf;
	struct rte_eth_txconf TxConf;
	static struct rte_eth_conf PortConf = {
    	.txmode = {
    		.mq_mode = RTE_ETH_MQ_TX_NONE,
    	},
    };
	struct rte_eth_dev_info DevInfo;
	struct rte_ether_addr EthAddress;
	
    Info->RxDescs = (Info->RxDescs ? Info->RxDescs : DEFAULT_RX_DESC);
	Info->TxDescs = (Info->TxDescs ? Info->TxDescs : DEFAULT_TX_DESC);
	
    uint32_t PortId = Info->Port;
	uint16_t RxDesc = Info->RxQueues * Info->RxDescs;
    uint16_t TxDesc = Info->TxQueues * Info->TxDescs;
	int Res = 0, NUMASocketId = rte_eth_dev_socket_id(PortId);
	
	printf("Initializing Device(port) %u...\n", PortId);
	
	if (!rte_eth_dev_is_valid_port(PortId))
        rte_exit(EXIT_FAILURE, "Device(port) %u is invalid.\n", PortId);

	Res = rte_eth_dev_info_get(PortId, &DevInfo);
	if (Res != 0)
        rte_exit(EXIT_FAILURE, "Cannot get Device(port) %u info: %s.\n", PortId, strerror(-Res));
		
	Res = rte_eth_dev_configure(PortId, Info->RxQueues, Info->TxQueues, &PortConf);
	if (Res < 0)
        rte_exit(EXIT_FAILURE, "Cannot configure device(port): err=%d, port=%u.\n", Res, PortId);
		
	Res = rte_eth_dev_adjust_nb_rx_tx_desc(PortId, &RxDesc, &TxDesc);
	if (Res < 0)
        rte_exit(EXIT_FAILURE, "Cannot adjust number of descriptors: err=%d, port=%u.\n", Res, PortId);
	
	Res = rte_eth_macaddr_get(PortId, &EthAddress);
	if (Res < 0)
        rte_exit(EXIT_FAILURE, "Cannot get MAC address: error info=%d, port=%u.\n", Res, PortId);

	fflush(stdout);
	
    TxConf = DevInfo.default_txconf;
    TxConf.offloads = PortConf.txmode.offloads;
    for (uint32_t Index = 0; Index < Info->TxQueues; ++Index) {
        Res = rte_eth_tx_queue_setup(PortId, Index, Info->TxDescs, NUMASocketId, &TxConf);
        if (Res < 0)
            rte_exit(EXIT_FAILURE, "Tx queue setup error: error info=%d, port=%u.\n", Res, PortId);
    }   

    RxConf = DevInfo.default_rxconf;
    RxConf.offloads = PortConf.rxmode.offloads;
    if (RxDesc) Info->Mempool = MemPoolAlloc(RxDesc);
    for (uint32_t Index = 0; Index < Info->RxQueues; ++Index) {
        Res = rte_eth_rx_queue_setup(PortId, Index, Info->RxDescs, NUMASocketId, &RxConf, Info->Mempool);
        if (Res < 0)
            rte_exit(EXIT_FAILURE, "Tx queue setup error: error info=%d, port=%u.\n", Res, PortId);
    }
	fflush(stdout);

	Res = rte_eth_dev_set_ptypes(PortId, RTE_PTYPE_UNKNOWN, NULL, 0);
	if (Res < 0)
        printf("Device(port) %u failed to disable Ptype parsing.\n", PortId);
		
	Res = rte_eth_dev_start(PortId);
	if (Res < 0)
        rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u.\n", Res, PortId);

	if (Info->Promiscuous) {
		Res = rte_eth_promiscuous_enable(PortId);
		if (Res != 0) rte_exit(EXIT_FAILURE, "Promiscuous enable false: error info=%s, port=%u.\n", rte_strerror(-Res), PortId);
	}
    
    printf("Finish init device(port) %u.\n", PortId);
    rte_ether_format_addr(MACAddress, RTE_ETHER_ADDR_FMT_SIZE, &EthAddress);
	printf("Device(port) %u\'s MAC address: %s.\n", PortId, MACAddress);

	return 0;
}
