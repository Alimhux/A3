#pragma once

#include <string>
#include <cstdint>
#include <functional>

class HashFuncGen {
public:
    HashFuncGen(uint32_t seed = 0);
    
    uint32_t hash(const std::string& key) const;
    
    std::function<uint32_t(const std::string&)> get_hash_function() const;
    
    void set_seed(uint32_t seed);
    uint32_t get_seed() const;

private:
    uint32_t seed_;
    
    static uint32_t murmur3_32(const std::string& key, uint32_t seed);
    static uint32_t rotl32(uint32_t x, int8_t r);
    static uint32_t fmix32(uint32_t h);
};
