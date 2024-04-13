#pragma once

#include <rte_ip.h>

#include <cstring>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <iostream>

#define IPV4_ADDR_FMT_SIZE 16
#define IPV6_ADDR_FMT_SIZE 40

extern "C" {
    void ParseInteger2IPv4(uint32_t IP, char* Str);
    int ParseIPv42Integer(const char* Str, uint32_t* IP);
    void ParseInteger2IPv6(uint8_t* IP, char* Str);
    int ParseIPv62Integer(const char* Str, uint8_t* IP);
} 
