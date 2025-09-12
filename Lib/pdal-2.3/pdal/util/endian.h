#pragma once
 
inline uint16_t ByteSwap(uint16_t v)
{
    return (v >> 8) | (v << 8);
}

inline uint32_t ByteSwap(uint32_t v)
{
    uint8_t b0 = v & 0xFF;
    uint8_t b1 = (v >> 8) & 0xFF;
    uint8_t b2 = (v >> 16) & 0xFF;
    uint8_t b3 = (v >> 24) & 0xFF;

    return (b0 << 24) | (b1 << 16) | (b2 << 8) | b3;
}

inline uint64_t ByteSwap(uint64_t v)
{
    uint64_t b0 = v & 0xFF;
    uint64_t b1 = (v >> 8) & 0xFF;
    uint64_t b2 = (v >> 16) & 0xFF;
    uint64_t b3 = (v >> 24) & 0xFF;

    uint64_t b4 = (v >> 32)  & 0xFF;
    uint64_t b5 = (v >> 40) & 0xFF;
    uint64_t b6 = (v >> 48) & 0xFF;
    uint64_t b7 = (v >> 56) & 0xFF;

    return
        (b0 << 56) | (b1 << 48) | (b2 << 40) | (b3 << 32) |
        (b4 << 24) | (b5 << 16) |  (b6 << 8) | b7;
}
