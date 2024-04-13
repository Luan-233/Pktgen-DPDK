#pragma once

#include <rte_mbuf.h>
#include <rte_lcore.h>
#include <rte_common.h>
#include <rte_mempool.h>

#include <atomic>
#include <cstdio>
#include <stdio.h>

#define MEMPOOL_CACHE_SIZE 32

extern "C" {
    struct rte_mempool* MemPoolAlloc(unsigned Number);
}
