#include "Memory.h"

struct rte_mempool* MemPoolAlloc(unsigned Number) {
    static std::atomic<int> Counter(0);
    struct rte_mempool* Ptr;
    char PoolName[16];
    sprintf(PoolName, "MemPool_%d", Counter.fetch_add(1));
    Number += MEMPOOL_CACHE_SIZE * rte_lcore_count();
    Number = RTE_MAX(Number, (1 << 8) -1);
    Ptr = rte_pktmbuf_pool_create(PoolName, Number, MEMPOOL_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
    if (!Ptr) rte_exit(EXIT_FAILURE, "Cannot init mbuf pool.\n");
    return Ptr;
}

