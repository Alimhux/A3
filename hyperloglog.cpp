#include "hyperloglog.h"
#include <cmath>
#include <algorithm>
#include <unordered_set>

HyperLogLog::HyperLogLog(uint8_t b) : b_(b), m_(1u << b) {
    if (b_ < 4 || b_ > 16) {
        b_ = 14;
        m_ = 1u << b_;
    }
    alpha_ = get_alpha(m_);
    registers_.resize(m_, 0);
}

double HyperLogLog::get_alpha(uint32_t m) {
    switch (m) {
        case 16:  return 0.673;
        case 32:  return 0.697;
        case 64:  return 0.709;
        default:  return 0.7213 / (1.0 + 1.079 / m);
    }
}

uint8_t HyperLogLog::count_leading_zeros(uint32_t value, uint8_t max_bits) {
    if (value == 0) return max_bits;
    
    uint8_t count = 0;
    while (count < max_bits && !(value & (1u << 31))) {
        value <<= 1;
        count++;
    }
    return count;
}

void HyperLogLog::add(uint32_t hash_value) {
    uint32_t idx = hash_value >> (32 - b_);
    uint32_t w = hash_value << b_;
    uint8_t rho = count_leading_zeros(w, 32 - b_) + 1;
    
    if (rho > registers_[idx]) {
        registers_[idx] = rho;
    }
}

void HyperLogLog::add_string(const std::string& s, 
                              std::function<uint32_t(const std::string&)> hash_func) {
    add(hash_func(s));
}

double HyperLogLog::estimate() const {
    double sum = 0.0;
    for (uint32_t i = 0; i < m_; ++i) {
        sum += std::pow(2.0, -static_cast<double>(registers_[i]));
    }
    
    double raw_estimate = alpha_ * m_ * m_ / sum;
    
    if (raw_estimate <= 2.5 * m_) {
        uint32_t zeros = 0;
        for (uint32_t i = 0; i < m_; ++i) {
            if (registers_[i] == 0) zeros++;
        }
        if (zeros > 0) {
            raw_estimate = m_ * std::log(static_cast<double>(m_) / zeros);
        }
    }
    
    if (raw_estimate > (1.0 / 30.0) * std::pow(2.0, 32)) {
        raw_estimate = -std::pow(2.0, 32) * std::log(1.0 - raw_estimate / std::pow(2.0, 32));
    }
    
    return raw_estimate;
}

void HyperLogLog::clear() {
    std::fill(registers_.begin(), registers_.end(), 0);
}

size_t HyperLogLog::count_exact(const std::vector<std::string>& stream) {
    std::unordered_set<std::string> unique_set(stream.begin(), stream.end());
    return unique_set.size();
}

size_t HyperLogLog::count_exact_at_position(const std::vector<std::string>& stream, size_t pos) {
    if (pos > stream.size()) pos = stream.size();
    std::unordered_set<std::string> unique_set(stream.begin(), stream.begin() + pos);
    return unique_set.size();
}
