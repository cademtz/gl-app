#include "fnv1a.hpp"

namespace fnv1a
{
    template <class T>
    T HashTemplate(size_t length, const uint8_t* data, T offset, T prime) {
        T hash = offset;
        for (const uint8_t* next = data; next < data + length; ++next) {
            hash ^= *next;
            hash *= prime;
        }
        return hash;
    }

    uint32_t Hash_32(size_t length, const uint8_t* data, uint32_t initial_hash) {
        return HashTemplate<uint32_t>(length, data, initial_hash, prime_32);
    }

    uint64_t Hash_64(size_t length, const uint8_t* data, uint32_t initial_hash) {
        return HashTemplate<uint64_t>(length, data, initial_hash, prime_64);
    }
}