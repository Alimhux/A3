#pragma once

#include <string>
#include <vector>
#include <random>
#include <cstdint>

class RandomStreamGen {
public:
    RandomStreamGen(uint64_t seed = 42);
    
    std::vector<std::string> generate(size_t total_elements, 
                                       size_t unique_count,
                                       size_t min_len = 5,
                                       size_t max_len = 30);
    
    std::vector<std::vector<std::string>> split_by_percentage(
        const std::vector<std::string>& stream,
        const std::vector<double>& percentages);
    
    std::vector<std::pair<size_t, std::vector<std::string>>> get_stream_at_steps(
        const std::vector<std::string>& stream,
        double step_percent = 10.0);

private:
    std::mt19937_64 rng_;
    static const std::string charset_;
    
    std::string generate_random_string(size_t min_len, size_t max_len);
};
