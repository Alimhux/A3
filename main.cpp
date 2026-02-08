#include "random_stream_gen.h"
#include "hash_func_gen.h"
#include "hyperloglog.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <unordered_set>

struct StreamConfig {
    size_t total_elements;
    size_t unique_count;
    std::string name;
};

struct ExperimentResult {
    int stream_id;
    std::string stream_name;
    int step;
    double percent;
    size_t processed;
    size_t f0_exact;
    double n_t_estimate;
    double relative_error;
};

void run_experiments(const std::vector<StreamConfig>& configs,
                     uint8_t b_value,
                     double step_percent,
                     const std::string& output_file,
                     int num_runs = 10) {
    
    std::ofstream csv(output_file);
    csv << "stream_id,stream_name,run_id,step,percent,processed,f0_exact,n_t_estimate,relative_error\n";
    
    HashFuncGen hash_gen(42);
    auto hash_func = hash_gen.get_hash_function();
    
    int stream_id = 0;
    for (const auto& config : configs) {
        std::cout << "Processing stream: " << config.name 
                  << " (total=" << config.total_elements 
                  << ", unique=" << config.unique_count << ")\n";
        
        for (int run = 0; run < num_runs; ++run) {
            RandomStreamGen gen(42 + run * 1000 + stream_id * 100);
            auto stream = gen.generate(config.total_elements, config.unique_count);
            
            HyperLogLog hll(b_value);
            
            int step = 0;
            for (double pct = step_percent; pct <= 100.0 + 0.001; pct += step_percent) {
                size_t target_pos = static_cast<size_t>(stream.size() * pct / 100.0);
                if (target_pos > stream.size()) target_pos = stream.size();
                
                size_t current_pos = (step == 0) ? 0 : 
                    static_cast<size_t>(stream.size() * (pct - step_percent) / 100.0);
                
                for (size_t i = current_pos; i < target_pos; ++i) {
                    hll.add_string(stream[i], hash_func);
                }
                
                size_t f0 = HyperLogLog::count_exact_at_position(stream, target_pos);
                double n_t = hll.estimate();
                double rel_error = (f0 > 0) ? (n_t - f0) / f0 : 0.0;
                
                csv << stream_id << ","
                    << config.name << ","
                    << run << ","
                    << step << ","
                    << std::fixed << std::setprecision(1) << pct << ","
                    << target_pos << ","
                    << f0 << ","
                    << std::setprecision(2) << n_t << ","
                    << std::setprecision(6) << rel_error << "\n";
                
                step++;
            }
        }
        stream_id++;
    }
    
    csv.close();
    std::cout << "Results saved to: " << output_file << "\n";
}

void test_hash_uniformity(const std::string& output_file, size_t num_samples = 100000) {
    std::ofstream csv(output_file);
    csv << "bucket,count\n";
    
    RandomStreamGen gen(12345);
    auto strings = gen.generate(num_samples, num_samples);
    
    HashFuncGen hash_gen(42);
    
    const int num_buckets = 256;
    std::vector<int> buckets(num_buckets, 0);
    
    for (const auto& s : strings) {
        uint32_t h = hash_gen.hash(s);
        int bucket = (h >> 24) & 0xFF;
        buckets[bucket]++;
    }
    
    for (int i = 0; i < num_buckets; ++i) {
        csv << i << "," << buckets[i] << "\n";
    }
    
    csv.close();
    std::cout << "Hash uniformity data saved to: " << output_file << "\n";
}

void test_different_b_values(const std::string& output_file) {
    std::ofstream csv(output_file);
    csv << "b_value,num_registers,run_id,f0_exact,n_t_estimate,relative_error,theoretical_error\n";
    
    HashFuncGen hash_gen(42);
    auto hash_func = hash_gen.get_hash_function();
    
    size_t total = 100000;
    size_t unique = 50000;
    
    for (uint8_t b = 4; b <= 16; ++b) {
        double theoretical_error = 1.04 / std::sqrt(1 << b);
        
        for (int run = 0; run < 20; ++run) {
            RandomStreamGen gen(42 + run * 1000);
            auto stream = gen.generate(total, unique);
            
            HyperLogLog hll(b);
            for (const auto& s : stream) {
                hll.add_string(s, hash_func);
            }
            
            size_t f0 = HyperLogLog::count_exact(stream);
            double n_t = hll.estimate();
            double rel_error = std::abs(n_t - f0) / f0;
            
            csv << static_cast<int>(b) << ","
                << (1 << b) << ","
                << run << ","
                << f0 << ","
                << std::fixed << std::setprecision(2) << n_t << ","
                << std::setprecision(6) << rel_error << ","
                << theoretical_error << "\n";
        }
    }
    
    csv.close();
    std::cout << "B-value comparison saved to: " << output_file << "\n";
}

int main() {

    std::cout << "=== HyperLogLog Experiment ===\n\n";
    
    std::cout << "1. Testing hash uniformity...\n";
    test_hash_uniformity("../files/hash_uniformity.csv");
    
    std::cout << "\n2. Testing different B values...\n";
    test_different_b_values("../files/b_value_comparison.csv");
    
    std::cout << "\n3. Running main experiments...\n";
    
    std::vector<StreamConfig> configs = {
        {10000,   5000,   "small_50pct"},
        {10000,   8000,   "small_80pct"},
        {100000,  50000,  "medium_50pct"},
        {100000,  80000,  "medium_80pct"},
        {500000,  250000, "large_50pct"},
        {500000,  400000, "large_80pct"},
        {1000000, 500000, "xlarge_50pct"},
    };
    
    uint8_t b_value = 14;
    double step_percent = 5.0;
    int num_runs = 10;
    
    run_experiments(configs, b_value, step_percent, "../files/hll_results.csv", num_runs);
    
    std::cout << "\n=== Experiment Complete ===\n";
    return 0;
}
