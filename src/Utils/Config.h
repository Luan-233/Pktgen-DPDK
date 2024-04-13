#pragma once

#include "Config.h"
#include "Memory.h"

#include <rte_ethdev.h>
#include <rte_mempool.h>

#include <stdio.h>
#include <signal.h>

#define DEFAULT_RX_DESC 512
#define DEFAULT_TX_DESC 512
#define MAX_ETH_PORT 256

struct EthConfigInfo {
    uint32_t Port; 
    struct rte_mempool* Mempool;
    uint16_t RxQueues;
	uint16_t TxQueues;
	uint16_t RxDescs;
	uint16_t TxDescs;
	bool Promiscuous;
};

extern "C" {
    int ConfigDevice(struct EthConfigInfo* Info, char* MACAddress);
}
