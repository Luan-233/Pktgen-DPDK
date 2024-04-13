#include "Format.h"

static int Decimal[1 << 7];

__attribute__((constructor)) void DecimalInit() {
    std::memset(Decimal, -1, sizeof(int) * (1 << 7));
    for (char X = '0'; X <= '9'; ++X) {
        Decimal[(uint32_t)X] = (int)(X - '0');
    }
    for (char X = 'a'; X <= 'f'; ++X) {
        Decimal[(uint32_t)X] = (int)(X - 'a') + 10;
    }
    for (char X = 'A'; X <= 'F'; ++X) {
        Decimal[(uint32_t)X] = (int)(X - 'A') + 10;
    }
}

static char Hexadecimal(uint32_t X) {
    return (X > 9) ? (X + '0') : (X - 10 + 'a');
}

void ParseInteger2IPv4(uint32_t IP, char* Str) {
    uint32_t Ptr = 0, Temp, Flag;
    for (uint32_t Index = 0; Index < 4; ++Index) {
        Temp = (IP >> (24 - (Index << 3))) & 0xff;
        Flag = 0;
        if (Temp / 100) {
            Str[Ptr++] = Temp / 100 + '0';
            Temp -= (Temp / 100) * 100;
            Flag = 1;
        }
        if ((Temp / 10) || Flag) {
            Str[Ptr++] = Temp / 10 + '0';
            Temp -= (Temp / 10) * 10;
            Flag = 1;
        }
        Str[Ptr++] = Temp + '0';
        Str[Ptr++] = '.';
    }
    Str[--Ptr] = 0;
}

int ParseIPv42Integer(const char* Str, uint32_t* IP) {
    uint16_t Addr[1 << 2];
    uint32_t Length = std::strlen(Str), Ptr = 0, Number = 0;
    for (uint32_t Index = 0; Index < 4; ++Index) {
        Number = 0, Addr[Index] = 0;
        while (isdigit(Str[Ptr])) {
            Addr[Index] = Addr[Index] * 10 + Decimal[(uint32_t)Str[Ptr++]];
            ++Number;
        }
        if ((Number > 3) || (Number == 0)) return -1;
        if (Index < 3) {
            if (Ptr > Length)  return -1;
            if (Str[Ptr] != '.')  return -1;
        }
        else if (Ptr != Length) return -1;
        ++Ptr;
    }
    (*IP) = RTE_IPV4(Addr[0], Addr[1], Addr[2], Addr[3]);
    return 0;
}

void ParseInteger2IPv6(uint8_t* IP, char* Str) {
    uint32_t Ptr = 0;
    for (uint32_t Index = 0; Index < 16; ++Index) {
        if (IP[Index] & 0xf0) {
            Str[Ptr++] = Hexadecimal(IP[Index] >> 4);
        }
        Str[Ptr++] = Hexadecimal(IP[Index] & 0x0f);
        if (Index & 1) Str[Ptr++] = ':';
    }
    Str[Ptr] = 0;
}

int ParseIPv62Integer(const char* Str, uint8_t* IP) {
    uint32_t Length = std::strlen(Str), Number = 0, Begin = 0, IPPtr = 0, Step = 0;
    uint16_t Value = 0;
    memset(IP, 0, sizeof(uint8_t) * 16);
    for (uint32_t Ptr = 1; Ptr < Length; ++Ptr) {
        if ((Str[Ptr] == ':') && (Str[Ptr - 1] == ':')) ++Number;
    }
    if (Number > 1) return -1;
    for (uint32_t Ptr = 0; Ptr < Length; ++Ptr) {
        if ((Str[Ptr] == ':') && (Str[Ptr - 1] == ':')) {
            Begin = IPPtr;
            continue;
        }
        Number = 0, Value = 0;
        while (Decimal[(uint32_t)Str[Ptr]] != -1) {
            Value = (Value << 4) | Decimal[(uint32_t)Str[Ptr++]];
            ++Number;
        }
        if (Number > 4) return -1;
        if ((Ptr < Length) && (Str[Ptr] != ':')) return -1;
        ++Ptr;
        IP[IPPtr++] = (Value >> 8) & 0x0f;
        IP[IPPtr++] = Value & 0x0f;
    }
    if (!Begin) return 0;
    Step = 15 - (--IPPtr);
    for (; IPPtr >= Begin; --IPPtr) std::swap(IP[IPPtr], IP[IPPtr + Step]);
    return 0;
}
