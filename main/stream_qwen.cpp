/**
 * @file demo_qwen.cpp
 * @author Chenghua Wang (chenghua.wang.edu@gmail.com)
 * @version 0.1
 * @date 2024-05-01
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "cmdline.h"
#include "models/qwen/configuration_qwen.hpp"
#include "models/qwen/modeling_qwen.hpp"
#include "models/qwen/tokenization_qwen.hpp"
#include "utils/json.hpp"
#include "hardware/dvfs.h"
#include "hardware/record.h"
#include "hardware/utils.h"

#include <cstdlib>
#include <thread>
#include <atomic>
#include <unistd.h>

using namespace mllm;
using namespace std;
using json = nlohmann::json;

std::atomic_bool sigterm(false);

int main(int argc, char **argv) {
    std::iostream::sync_with_stdio(false);
    cmdline::parser cmdParser;
    pid_t pid = getpid();

    // arg parser: BASIC
    cmdParser.add<string>("vocab", 'v', "specify mllm tokenizer model path", false, "../vocab/qwen_vocab.mllm");
    cmdParser.add<string>("merge", 'e', "specify mllm merge file path", false, "../vocab/qwen_merges.txt");
    cmdParser.add<string>("model", 'm', "specify mllm model path", false, "../models/qwen-1.5-1.8b-q8_0.mllm");
    cmdParser.add<string>("billion", 'b', "[0.5B | 1.8B | 1.5B |]", false, "1.8B");
    cmdParser.add<string>("family", 'f', "[Qwen1.5 | Qwen2.5 |]", false, "Qwen1.5");
    cmdParser.add<int>("limits", 'l', "max KV cache size", false, 1024);
    cmdParser.add<bool>("strict", 0, "apply token limits to only output tokens", false, false);
    cmdParser.add<int>("thread", 't', "num of threads", false, 4);

    // arg parser: For Stream
    cmdParser.add<bool>("interface", 'i', "print inference interface", false, true);
    cmdParser.add<int>("start", 's', "starting num of queries", false, -1);
    cmdParser.add<int>("length", 'L', "num of queries", false, -1);
    cmdParser.add<string>("input", 'I', "input dataset path of csv. ex) ./dataset/input.csv", false, ".");
    cmdParser.add<string>("output", 'O', "output directory path. ex) ./output/", false, ".");
    cmdParser.add<bool>("save", 'S', "save query-answer pairs with json", false, true);

    // arg parser: For DVFS
    cmdParser.add<string>("device", 'D', "specify your android phone [Pixel9 | S24]", true, "");
    cmdParser.add<int>("cpu-p", 'c', "specify CPU clock index for CPU DVFS", true, 0);
    cmdParser.add<int>("ram-p", 'r', "specify RAM clock index for RAM DVFS", true, 0);
    cmdParser.add<int>("cpu-d", 'C', "specify CPU clock index for CPU DVFS", true, 0);
    cmdParser.add<int>("ram-d", 'R', "specify RAM clock index for RAM DVFS", true, 0);

    // arg parser: For IGNITE techniques
    /* Unit [ms] */
    cmdParser.add<int>("phase-pause", 'p', "specify a pause time between phases (ms)", true, 0);
    cmdParser.add<int>("token-pause", 'P', "specify a pause time between generation tokens (ms)", false, 0);
    cmdParser.add<int>("layer-pause", 'y', "specify a pause time between self-attention layers during prefill (ms)", true, 0);
    /* Unit [s] */
    cmdParser.add<int>("query-interval", 'q', "specify an interval time between queries (s)", true, 0);
    cmdParser.parse_check(argc, argv);

    // variable initialization: BASIC
    string vocab_path = cmdParser.get<string>("vocab");
    string merge_path = cmdParser.get<string>("merge");
    string model_path = cmdParser.get<string>("model");
    string model_billion = cmdParser.get<string>("billion");
    string model_family = cmdParser.get<string>("family");
    int tokens_limit = cmdParser.get<int>("limits");
    CPUBackend::cpu_threads = cmdParser.get<int>("thread");
    bool strict_limit = cmdParser.get<bool>("strict");

    // variable initialization: For DVFS
    const string device_name = cmdParser.get<string>("device");
    const int cpu_clk_idx_p = cmdParser.get<int>("cpu-p");
    const int ram_clk_idx_p = cmdParser.get<int>("ram-p");
    const int cpu_clk_idx_d = cmdParser.get<int>("cpu-d");
    const int ram_clk_idx_d = cmdParser.get<int>("ram-d");

    // variable initialization: For Stream
    const bool interface = cmdParser.get<bool>("interface");
    const int qa_start = cmdParser.get<int>("start");
    const int qa_len = cmdParser.get<int>("length");
    const string input_path = cmdParser.get<string>("input");
    const string output_dir = cmdParser.get<string>("output"); //"HotpotQA_mllm_result_Qwen"+model_billion+".json";
    const bool is_query_save = cmdParser.get<bool>("save");
    int qa_now = qa_start;
    int qa_limit = 0;
    string output_hard;
    string output_infer;
    string output_qa;

    // variable initialization: For Pause Techniques
    int token_pause = cmdParser.get<int>("token-pause");
    int phase_pause = cmdParser.get<int>("phase-pause");
    int layer_pause = cmdParser.get<int>("layer-pause");
    int query_interval = cmdParser.get<int>("query-interval") * 1000;

    // variable initialization: For File Naming
    bool fixed_config = (cpu_clk_idx_p == cpu_clk_idx_d) && (ram_clk_idx_p == ram_clk_idx_d);
    bool tp = (token_pause > 0);
    bool pp = (phase_pause > 0);
    bool lp = (layer_pause > 0);
    char mode = 0b00000000; // 1byte

    // 0-th bit: clock control
    // 1-th bit: phase-pause
    // 2-th bit: layer-pause
    // 3-th bit: token-pause
    // ex) 0b0101 : clock config control + layer pause
    //     3 <-> 0

    // [control mode checker]
    mode |= (!fixed_config) ? (1 << 0) : 0;
    mode |= pp ? (1 << 1) : 0;
    mode |= lp ? (1 << 2) : 0;
    mode |= tp ? (1 << 3) : 0;

    switch (mode) {
    case 0:
        // Fixed Config
        output_hard = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_hard.txt");
        output_infer = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_infer.txt");
        break;
    case 1:
        // Only Config Control
        output_hard = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_to_" + to_string(cpu_clk_idx_d) + "-" + to_string(ram_clk_idx_d) + "_hard.txt");
        output_infer = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_to_" + to_string(cpu_clk_idx_d) + "-" + to_string(ram_clk_idx_d) + "_infer.txt");
        break;
    case 2:
        // Only Phase Pause
        output_hard = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_pp_" + to_string(phase_pause) + "_hard.txt");
        output_infer = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_pp_" + to_string(phase_pause) + "_infer.txt");
        break;
    case 4:
        // Only Layer Pause
        output_hard = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_lp_" + to_string(layer_pause) + "_hard.txt");
        output_infer = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_lp_" + to_string(layer_pause) + "_infer.txt");
        break;
    case 5:
        // Config Control + Layer Pause
        output_hard = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_to_" + to_string(cpu_clk_idx_d) + "-" + to_string(ram_clk_idx_d) + "_lp_" + to_string(layer_pause) + "_hard.txt");
        output_infer = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_to_" + to_string(cpu_clk_idx_d) + "-" + to_string(ram_clk_idx_d) + "_lp_" + to_string(layer_pause) + "_infer.txt");
        break;
    case 8:
        // Only Token Pause
        output_hard = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_tp_" + to_string(token_pause) + "_hard.txt");
        output_infer = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_" + to_string(cpu_clk_idx_p) + "-" + to_string(ram_clk_idx_p) + "_tp_" + to_string(token_pause) + "_infer.txt");
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
        cerr << "[ERROR] Not Controlled Configuration\n";
        return 1;
    }
    output_qa = joinPaths(output_dir, "HotpotQA_mllm_Qwen_" + model_billion + "_result.json");

    // variable initialization: For Thermal Throttling Detection
    std::string command = "su -c \"";                                                            // prefix
    command += "awk '{print \\$1/1000}' /sys/devices/system/cpu/cpu7/cpufreq/scaling_cur_freq;"; // command
    command += "\"";                                                                             // postfix

    // Model Configuration
    auto tokenizer = QWenTokenizer(vocab_path, merge_path);
    QWenConfig config(
        strict_limit ? tokens_limit + 8192 : tokens_limit, 
        model_billion, RoPEType::HFHUBROPE, model_family);
    auto model = QWenForCausalLM(config);
    model.load(model_path);
    Module::thread_sleep = layer_pause; // set layer-pause time

    // QA Dataset Load
    vector<vector<string>> qa_list = readCSV(input_path); // qa load
    vector<string> ans;                                   // qa load

    // DVFS setting
    DVFS dvfs(device_name);
    dvfs.output_filename = output_hard; // dvfs.output_filename requires hardware recording output path
    if (dvfs.init_fd_cache() != 0) {
        fprintf(stderr, "FD cache initialization failed. Are you root or authorized?\n");
    }

    cout << pid << "\r\n";
    vector<int> freq_config = dvfs.get_cpu_freqs_conf(cpu_clk_idx_p);
    for (auto f : freq_config) { cout << f << " "; }
    cout << "\r\n"; // to validate (print freq-configuration)

    const vector<string> infer_record_names = {"sys_time", "load_time", "prefill_speed", "decode_speed", "prefill_token", "decode_token", "ttft"};
    write_file(infer_record_names, output_infer);

    // limit=-1 -> infinite query stream
    if (qa_len == -1) {
        qa_limit = qa_list.size();
    } else {
        qa_limit = MIN(qa_list.size(), qa_start + qa_len) - 1;
    }

    // measurement start
    auto start_sys_time = chrono::system_clock::now();
    std::thread record_thread = std::thread(record_hard, std::ref(sigterm), dvfs);

    while ((qa_now - qa_start) < qa_limit) {
        string question = qa_list[qa_now][1];
        string answer;
        int ft = 0; // first token
        auto now_sys_time = chrono::system_clock::now();

        freq_config = dvfs.get_cpu_freqs_conf(cpu_clk_idx_p);
        dvfs.set_cpu_freq(freq_config);
        dvfs.set_ram_freq(ram_clk_idx_p);

        //auto time1 = chrono::system_clock::now();
        auto input_str = tokenizer.apply_chat_template(question);
        auto input_tensor = tokenizer.tokenize(input_str);
        //auto time2 = chrono::system_clock::now();
        // std::cout << chrono::duration_cast<chrono::microseconds>(time2 - time1).count() << " " << input_tensor.sequence() << std::endl; // test
        if (interface) {
            std::cout << "[Q] " << question << "\r\n";
            std::cout << "[A] " << std::flush;
        }

        // INFERENCE
        std::size_t max_new_tokens = strict_limit ? tokens_limit : tokens_limit - input_tensor.sequence();
        //size_t max_new_tokens = 256;
        LlmTextGeneratorOpts opt{
            .max_new_tokens = max_new_tokens,
            .do_sample = false,
            .temperature = 0.0F};
        model.generate(input_tensor, opt, [&](unsigned int out_token) -> bool {
            auto out_string = tokenizer.detokenize({out_token});
            // now prefill done (when ft==0)

            // phase clock control
            if (ft == 0 && !fixed_config) {
                freq_config = dvfs.get_cpu_freqs_conf(cpu_clk_idx_d);
                dvfs.set_cpu_freq(freq_config);
                dvfs.set_ram_freq(ram_clk_idx_d);
            }
            // phase pause
            if (ft == 0 && phase_pause > 0) {
                // std::cout << std::flush << "sleep\n"; // test
                this_thread::sleep_for(chrono::milliseconds(phase_pause));
            }

            // generation start
            auto [not_end, output_string] = tokenizer.postprocess(out_string);
            if (!not_end) { return false;} 
            else {
                // std::cout << std::flush << " tp "; // test
                this_thread::sleep_for(chrono::milliseconds(token_pause)); // token pause
            }

            // interface support
            if (interface) {
                std::cout << output_string << std::flush;
                output_string.erase(std::remove(output_string.begin(), output_string.end(), '\0'), output_string.end());
            }
            answer += output_string;
            ft++;
            return true;
        });
        std::cout << "\r\n";

        // store data
        auto sys_time = chrono::duration_cast<chrono::milliseconds>(now_sys_time - start_sys_time).count();
        // proifle_res = { prefill_speed, decode_speed, input_token, output_token, ttft  }
        auto profile_res = model.profiling("Inference");
        profile_res.insert(profile_res.begin(), (double)sys_time / (double)1000.0);
        write_file(profile_res, output_infer); // store in real time

        // Throttling detection
        // single query is done
        // This throttling detection is valid for only Pixel9
        int cur_cpu_freq = stoi(split_string(execute_cmd(command.c_str()))[0]);
        if (cur_cpu_freq * 1000 != dvfs.get_cpu_freq().at(7).at(freq_config[2])) {
            Module::thread_sleep = 0; // reset layer-pause
            phase_pause = 0;          // reset phase-pause
            token_pause = 0;          // reset token-pause
        }

        // Reset
        if (is_query_save) { ans.push_back(answer); } // accummulate answers
        model.clear_kvcache();
        qa_now++;
        ft = 0;

        // Query-interval
        if((qa_now - qa_start) < qa_limit) this_thread::sleep_for(chrono::milliseconds(query_interval));
    }

    // measurement done
    sigterm = true;
    dvfs.unset_cpu_freq();
    dvfs.unset_ram_freq();
    record_thread.join();

    cout << "DONE\r\n";
    this_thread::sleep_for(chrono::milliseconds(1000));

    // post-measurement-processing
    if (!is_query_save) {
        // not save the result
        // system("input touchscreen keyevent 26");
        // system("input touchscreen keyevent 82");
        return 0;
    }

    /* STREAM RESULT */
    // store answers
    json qa_array = json::array();
    for (int i = 0; i < ans.size(); ++i) {
        json pair;
        pair["question"] = qa_list[qa_start + i][1];
        pair["answer"] = ans[i];
        qa_array.push_back(pair);
    }

    std::ofstream output_file(output_qa); // open file stream
    if (!output_file) {
        std::cerr << "Error: Could not open file for writing: " << output_qa << std::endl;
        return 1;
    }

    output_file << qa_array.dump(4); // pretty print with indent=4
    output_file.close();             // close file stream
    std::cout << "Saved " << ans.size() << " QA pairs to " << output_qa << "\r\n";

    // termination notification
    // system("input touchscreen keyevent 26");
    // system("input touchscreen keyevent 82");
    return 0;
}