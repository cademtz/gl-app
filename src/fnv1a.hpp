#include <cstdint>
#include <cstddef>

namespace fnv1a
{
    static const uint32_t offset_32 = 0x811c9dc5;
    static const uint32_t prime_32 = 0x1000193;
    static const uint64_t offset_64 = 0xcbf29ce484222325;
    static const uint64_t prime_64 = 0x100000001b3;

    uint32_t Hash_32(size_t length, const uint8_t* data, uint32_t initial_hash = offset_32);
    
    template <class T>
    uint32_t Hash_32(const T& value, uint32_t initial_hash = offset_32) {
        return Hash_32(sizeof(value), (const uint8_t*)&value, initial_hash);
    }

    uint64_t Hash_64(size_t length, const uint8_t* data, uint64_t initial_hash = offset_64);
    
    template <class T>
    uint64_t Hash_64(const T& value, uint64_t initial_hash = offset_64) {
        return Hash_64(sizeof(value), (const uint8_t*)&value, initial_hash);
    }
}