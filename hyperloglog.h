#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>

class HyperLogLog {
public:
    explicit HyperLogLog(uint8_t b = 14);
    
    void add(uint32_t hash_value);
    void add_string(const std::string& s, std::function<uint32_t(const std::string&)> hash_func);
    
    double estimate() const;
    
    void clear();
    
    uint8_t get_b() const { return b_; }
    uint32_t get_m() const { return m_; }
    const std::vector<uint8_t>& get_registers() const { return registers_; }
    
    static size_t count_exact(const std::vector<std::string>& stream);
    static size_t count_exact_at_position(const std::vector<std::string>& stream, size_t pos);

private:
    uint8_t b_;
    uint32_t m_;
    double alpha_;
    std::vector<uint8_t> registers_;
    
    static uint8_t count_leading_zeros(uint32_t value, uint8_t max_bits);
    static double get_alpha(uint32_t m);
};
