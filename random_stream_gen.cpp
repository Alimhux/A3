#include "random_stream_gen.h"
#include <algorithm>
#include <unordered_set>

const std::string RandomStreamGen::charset_ = 
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";

RandomStreamGen::RandomStreamGen(uint64_t seed) : rng_(seed) {}

std::string RandomStreamGen::generate_random_string(size_t min_len, size_t max_len) {
    std::uniform_int_distribution<size_t> len_dist(min_len, max_len);
    std::uniform_int_distribution<size_t> char_dist(0, charset_.size() - 1);
    
    size_t length = len_dist(rng_);
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += charset_[char_dist(rng_)];
    }
    return result;
}

std::vector<std::string> RandomStreamGen::generate(size_t total_elements, 
                                                    size_t unique_count,
                                                    size_t min_len,
                                                    size_t max_len) {
    if (unique_count > total_elements) {
        unique_count = total_elements;
    }
    
    std::unordered_set<std::string> unique_set;
    std::vector<std::string> unique_strings;
    unique_strings.reserve(unique_count);
    
    while (unique_strings.size() < unique_count) {
        std::string s = generate_random_string(min_len, max_len);
        if (unique_set.find(s) == unique_set.end()) {
            unique_set.insert(s);
            unique_strings.push_back(s);
        }
    }
    
    std::vector<std::string> stream;
    stream.reserve(total_elements);
    
    for (size_t i = 0; i < unique_count; ++i) {
        stream.push_back(unique_strings[i]);
    }
    
    std::uniform_int_distribution<size_t> idx_dist(0, unique_count - 1);
    for (size_t i = unique_count; i < total_elements; ++i) {
        stream.push_back(unique_strings[idx_dist(rng_)]);
    }
    
    std::shuffle(stream.begin(), stream.end(), rng_);
    return stream;
}

std::vector<std::vector<std::string>> RandomStreamGen::split_by_percentage(
    const std::vector<std::string>& stream,
    const std::vector<double>& percentages) {
    
    std::vector<std::vector<std::string>> result;
    result.reserve(percentages.size());
    
    for (double pct : percentages) {
        size_t count = static_cast<size_t>(stream.size() * pct / 100.0);
        result.emplace_back(stream.begin(), stream.begin() + count);
    }
    return result;
}

std::vector<std::pair<size_t, std::vector<std::string>>> RandomStreamGen::get_stream_at_steps(
    const std::vector<std::string>& stream,
    double step_percent) {
    
    std::vector<std::pair<size_t, std::vector<std::string>>> result;
    
    for (double pct = step_percent; pct <= 100.0; pct += step_percent) {
        size_t count = static_cast<size_t>(stream.size() * pct / 100.0);
        if (count > stream.size()) count = stream.size();
        result.emplace_back(count, std::vector<std::string>(stream.begin(), stream.begin() + count));
    }
    return result;
}
