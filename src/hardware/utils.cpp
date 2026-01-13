#include "utils.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> values;
    std::string current;
    bool insideQuotes = false;

    for (char ch : line) {
        if (ch == '"') {
            insideQuotes = !insideQuotes;
        } else if (ch == ',' && !insideQuotes) {
            values.push_back(current);
            current.clear();
        } else {
            current += ch;
        }
    }
    values.push_back(current); // last field

    return values;
}

std::vector<std::vector<std::string>> readCSV(const std::string& filename) {
    std::vector<std::vector<std::string>> result;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "cannot open file: " << filename << std::endl;
        return result;
    }

    std::string line; std::size_t i = 0;
    while (std::getline(file, line)) {
        result.push_back(parseCSVLine(line));
    }

    file.close();
    return result;
}

std::string joinPaths(const std::string& path1, const std::string& path2) {
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;
    
    char lastChar = path1[path1.length() - 1];
    char firstChar = path2[0];
    
    if (lastChar == '/' && firstChar == '/') {
        return path1 + path2.substr(1);
    } else if (lastChar != '/' && firstChar != '/') {
        return path1 + "/" + path2;
    } else {
        return path1 + path2;
    }
}

std::string replace(std::string origin, std::string target, std::string destination) {
    size_t pos = 0;
    while ((pos = origin.find(target, pos)) != std::string::npos) {
        origin.replace(pos, target.length(), destination);
        pos += destination.length();
    }
    return origin;
}

// -------------------------------------------
// TODO: move to utils (refactoring)
std::vector<std::string> split_string(const std::string& str){
    // initialization
    std::vector<std::string> result;
    // conversion to stream string
    std::istringstream iss(str);
    std::string value; // splited value

    while (iss >> value){
        result.push_back(value); // accumulation
    }

    return result;
}


std::string execute_cmd(const char* cmd){
    // command execution
    FILE* pipe = popen(cmd, "r");
    
    // check pipe open
    if (!pipe) { std::cerr << "failed to pipe open (record.h)\n"; return "";}

    // get output from buffer
    std::ostringstream result;
    char buff[8192];
    while (fgets(buff, sizeof(buff), pipe) != nullptr){
        result << buff;
    }

    // close pipe
    pclose(pipe); 
    return result.str();
}

// throttling detection support
std::string apply_sudo_and_get(std::string command = "") {
    std::string cmd = "su -c \"";                                                                 // prefix
    if (command != "")  cmd += command;
    else cmd += "awk '{print \\$1/1000}' /sys/devices/system/cpu/cpu7/cpufreq/scaling_cur_freq;"; // command
    cmd += "\"";                                                                                  // postfix

    return cmd;
}
// -------------------------------------------

const bool init_ignite_filename(ignite_params& _ip){
    // variable initialization: For File Naming
    bool fixed_config = (_ip.cpu_clk_idx_p == _ip.cpu_clk_idx_d) && (_ip.ram_clk_idx_p == _ip.ram_clk_idx_d);
    bool tp = (_ip.token_pause > 0);
    bool pp = (_ip.phase_pause > 0);
    bool lp = (_ip.layer_pause > 0);
    bool qi = (_ip.query_interval > 0);
    char mode = 0b00000000; // 1byte

    // 0-th bit: clock control
    // 1-st bit: phase-pause
    // 2-nd bit: layer-pause
    // 3-rd bit: token-pause
    // 4-th bit: query-interval
    // ex) 0b0101 : clock config control + layer pause
    //     3 <-> 0

    // [control mode checker]
    mode |= (!fixed_config) ? (1 << 0) : 0;
    mode |= pp ? (1 << 1) : 0;
    mode |= lp ? (1 << 2) : 0;
    mode |= tp ? (1 << 3) : 0;
    mode |= qi ? (1 << 4) : 0;

    std::string output_hard;
    std::string output_infer;
    switch (mode) {
    case 0:
        // Fixed Config
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_infer.txt");
        break;
    case 1:
        // Only Config Control
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_to_" + std::to_string(_ip.cpu_clk_idx_d) + "-" + std::to_string(_ip.ram_clk_idx_d) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_to_" + std::to_string(_ip.cpu_clk_idx_d) + "-" + std::to_string(_ip.ram_clk_idx_d) + "_infer.txt");
        break;
    case 2:
        // Only Phase Pause
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_pp_" + std::to_string(_ip.phase_pause) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_pp_" + std::to_string(_ip.phase_pause) + "_infer.txt");
        break;
    case 4:
        // Only Layer Pause
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_lp_" + std::to_string(_ip.layer_pause) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_lp_" + std::to_string(_ip.layer_pause) + "_infer.txt");
        break;
    case 5:
        // Config Control + Layer Pause
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_to_" + std::to_string(_ip.cpu_clk_idx_d) + "-" + std::to_string(_ip.ram_clk_idx_d) + "_lp_" + std::to_string(_ip.layer_pause) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_to_" + std::to_string(_ip.cpu_clk_idx_d) + "-" + std::to_string(_ip.ram_clk_idx_d) + "_lp_" + std::to_string(_ip.layer_pause) + "_infer.txt");
        break;
    case 8:
        // Only Token Pause
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_tp_" + std::to_string(_ip.token_pause) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_tp_" + std::to_string(_ip.token_pause) + "_infer.txt");
        break;
    case 16:
        // Only query interval
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_qi_" + std::to_string(_ip.query_interval) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_qi_" + std::to_string(_ip.query_interval) + "_infer.txt");
        break;
    case 17:
        // Config Control + Query Interval
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_to_" + std::to_string(_ip.cpu_clk_idx_d) + "-" + std::to_string(_ip.ram_clk_idx_d) + "_qi_" + std::to_string(_ip.query_interval) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_to_" + std::to_string(_ip.cpu_clk_idx_d) + "-" + std::to_string(_ip.ram_clk_idx_d) + "_qi_" + std::to_string(_ip.query_interval) + "_infer.txt");
        break;
    case 20:
        // Layer Pause + Query Interval
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_lp_" + std::to_string(_ip.layer_pause) + "_qi_" + std::to_string(_ip.query_interval) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_lp_" + std::to_string(_ip.layer_pause) + "_qi_" + std::to_string(_ip.query_interval) + "_infer.txt");
        break;
    case 21:
        // Config Control + Layer Pause + Query Interval
        output_hard = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_to_" + std::to_string(_ip.cpu_clk_idx_d) + "-" + std::to_string(_ip.ram_clk_idx_d) + "_lp_" + std::to_string(_ip.layer_pause) + "_qi_" + std::to_string(_ip.query_interval) + "_hard.txt");
        output_infer = joinPaths(_ip.output_dir, "HotpotQA_mllm_Qwen3_" + _ip.model_billion + "_" + std::to_string(_ip.cpu_clk_idx_p) + "-" + std::to_string(_ip.ram_clk_idx_p) + "_to_" + std::to_string(_ip.cpu_clk_idx_d) + "-" + std::to_string(_ip.ram_clk_idx_d) + "_lp_" + std::to_string(_ip.layer_pause) + "_qi_" + std::to_string(_ip.query_interval) + "_infer.txt");
        break;
    
    case 3:  // Config Control + Phase Pause
    case 6:  // Phase Pause + Layer Pause
    case 7:  // Config Control + Pase Pause + Layer Pause
    case 9:  // Config Control + Token Pause
    case 10: // Phase Pause + Token Pause
    case 11: // Config Control + Phase Pause + Token Pause
    case 12: // Layer Pause + Token Pause
    case 13: // Config Control + Layer Pause + Token Pause
    case 14: // Phase Pause + Layer Pause + Token Pause
    case 15: // Config Control + Phase Pause + Layer Pause + Token Pause
    default:
        // Not controlled cases
        std::cerr << "[ERROR] Not Controlled Configuration\n";
        return false;
    }

    _ip.fixed_config = fixed_config;
    _ip.output_path_hard = output_hard;
    _ip.output_path_infer = output_infer;

    return true;
}