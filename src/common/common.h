#ifndef COMMON_H
#define COMMON_H

// This common file is for IGNITE technique parameters

#include <vector>
#include <string>

struct ignite_params {
    // basic model configs
    std::string vocab_path = "";
    std::string merge_path = "";
    std::string model_path = "";
    std::string model_billion = "";
    std::string model_family = "";
    int tokens_limit = 0;
    bool strict_limit = false;
    bool enable_thinking = false;

    // llm plane
    int phase_pause = 0; // ms
    int token_pause = 0; // ms
    int layer_pause = 0; // ms
    int query_interval = 0; // ms
    bool prefill_phase = true; // prefill phase or not
    double prefill_speed = 0.0; // tokens/s
    double decode_speed = 0.0; // tokens/s

    // basic measure configs
    std::string input_path = ""; // path = dir/file.ext
    std::string output_dir = "";
    std::string output_path_hard = "";
    std::string output_path_infer = "";
    
    // [OPT. 1] resource plane (static ignite)
    int cpu_clk_idx_p = 0; // prefill + cpu
    int ram_clk_idx_p = 0; // prefill + ram
    int cpu_clk_idx_d = 0; // decode + cpu
    int ram_clk_idx_d = 0; // decode + ram
    bool fixed_config = false;

    // [OPT. 2] resource plane (agent ignite)
    double time_slot = 0.5; // s
    double temp_threshold = 80.0; // Celsius
    std::vector<double> temp_history = {}; // temperature history
    int temp_cap = 10; // max length of temperature history
    double temp_alpha = 0.6; // for EMA
    int max_cpu_clk_idx = 0; // fixed by device
    int cur_cpu_clk_idx = 0; // dynamic
    int max_ram_clk_idx = 0; // fixed by device
    int cur_ram_clk_idx = 0; // dynamic
};

#endif // COMMON_H