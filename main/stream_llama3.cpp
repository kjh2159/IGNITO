/**
 * @file stream_llama.cpp
 * @author Jinhwi Kim (kjh2159@dgist.ac.kr)
 * @version 0.1
 * @date 2025-05-12
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "cmdline.h"
#include "models/llama3/modeling_llama3.hpp"
#include "models/llama3/tokenization_llama3.hpp"
#include "processor/PostProcess.hpp"
#include "utils/json.hpp"
#include "hardware/dvfs.h"
#include "hardware/record.h"
#include "hardware/utils.h"
#include "common/common.h" // common for ignite

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
    ignite_params _params;

    // arg parser: BASIC
    cmdParser.add<string>("vocab", 'v', "specify mllm tokenizer model path", false, "../vocab/llama3_tokenizer.model");
    cmdParser.add<string>("model", 'm', "specify mllm model path", false, "../models/llama-3.2-1b-instruct_q4_k.mllm");
    cmdParser.add<string>("billion", 'b', "[1B | 3B ]", false, "1B");
    cmdParser.add<int>("limits", 'l', "max KV cache size", false, 1024);
    cmdParser.add<int>("thread", 't', "num of threads", false, 4);
    cmdParser.add<bool>("strict", 0, "apply token limits to only output tokens", false, false);
    
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
    _params.vocab_path = cmdParser.get<string>("vocab");
    _params.model_path = cmdParser.get<string>("model");
    _params.model_billion = cmdParser.get<string>("billion");
    _params.tokens_limit = cmdParser.get<int>("limits");
    CPUBackend::cpu_threads = cmdParser.get<int>("thread");
    _params.strict_limit = cmdParser.get<bool>("strict");


    // variable initialization: For DVFS
    const string device_name = cmdParser.get<string>("device");
    _params.cpu_clk_idx_p = cmdParser.get<int>("cpu-p");
    _params.ram_clk_idx_p = cmdParser.get<int>("ram-p");
    _params.cpu_clk_idx_d = cmdParser.get<int>("cpu-d");
    _params.ram_clk_idx_d = cmdParser.get<int>("ram-d");


    // variable initialization: For Stream
    const bool interface = cmdParser.get<bool>("interface");
    const int qa_start = cmdParser.get<int>("start");
    const int qa_len = cmdParser.get<int>("length");
    const bool is_query_save = cmdParser.get<bool>("save");
    _params.input_path = cmdParser.get<string>("input");
    _params.output_dir = cmdParser.get<string>("output"); //"HotpotQA_mllm_result_Qwen"+model_billion+".json";
    int qa_now = qa_start;
    int qa_limit = 0;
    string output_qa;

    // variable initialization: For Pause Techniques
    _params.token_pause = cmdParser.get<int>("token-pause");
    _params.phase_pause = cmdParser.get<int>("phase-pause");
    _params.layer_pause = cmdParser.get<int>("layer-pause");
    _params.query_interval = cmdParser.get<int>("query-interval") * 1000;

    // file path initialization
    if (!init_ignite_filename(_params)) return -1; // when failed
    output_qa = joinPaths(_params.output_dir, "HotpotQA_mllm_Qwen_" + _params.model_billion + "_result.json");

    // variable initialization: For Thermal Throttling Detection
    std::string command = apply_sudo_and_get(""); // this function get the command of cpu clock


    // Model Configuration
    Llama3Config config(
        _params.strict_limit ? _params.tokens_limit + 8192 : _params.tokens_limit, 
        _params.model_billion);
    auto tokenizer = LLama3Tokenizer(_params.vocab_path);
    config.cache_limit = _params.tokens_limit;
    auto model = Llama3Model(config);
    model.load(_params.model_path);
    Module::thread_sleep = _params.layer_pause; // set layer-pause time

    // QA Dataset Load
    vector<vector<string>> qa_list = readCSV(_params.input_path); // qa load
    vector<string> ans; // qa load

    // DVFS setting
    DVFS dvfs(device_name);
    dvfs.output_filename = _params.output_path_hard; // dvfs.output_filename requires hardware recording output path
    if (dvfs.init_fd_cache() != 0) {
        fprintf(stderr, "FD cache initialization failed. Are you root or authorized?\n");
    }

    cout << pid << "\r\n";
    vector<int> freq_config = dvfs.get_cpu_freqs_conf(_params.cpu_clk_idx_p);
    for (auto f : freq_config) { cout << f << " "; }
    cout << "\r\n"; // to validate (print freq-configuration)
    
    const vector<string> infer_record_names = {"sys_time", "load_time", "prefill_speed", "decode_speed", "prefill_token", "decode_token", "ttft"};
    write_file(infer_record_names, _params.output_path_infer);

    // limit=-1 -> infinite query stream
    if (qa_len == -1) {
        qa_limit = qa_list.size();
    } else {
        qa_limit = MIN(qa_list.size(), qa_start + qa_len) - 1;
    }
    
    // Hardware Measurement start
    auto start_sys_time = chrono::system_clock::now();
    std::thread record_thread = std::thread(record_hard, std::ref(sigterm), std::ref(dvfs));
    bool throttling = false;

    // Execution
    while ( (qa_now - qa_start) < qa_limit ) {
        string question = qa_list[qa_now][1];
        string answer;
        int ft = 0; // first token
        auto now_sys_time = chrono::system_clock::now();

        freq_config = dvfs.get_cpu_freqs_conf(_params.cpu_clk_idx_p);
        dvfs.set_cpu_freq(freq_config);
        dvfs.set_ram_freq(_params.ram_clk_idx_p);
        
        // Input Tokenization
        auto input_str = tokenizer.apply_chat_template(question);
        auto input_tensor = tokenizer.tokenize(input_str);
        if (interface){
            std::cout << "[Q] " << question << "\r\n";
            std::cout << "[A] " << std::flush;
        }

        // INFERENCE
        std::size_t max_new_tokens = _params.strict_limit ? _params.tokens_limit :  _params.tokens_limit - input_tensor.sequence();
        //size_t max_new_tokens = 256;
        LlmTextGeneratorOpts opt{
            .max_new_tokens = max_new_tokens,
            .do_sample = false,
            .temperature = 0.0F
        };
        model.generate(input_tensor, opt, [&](unsigned int out_token) -> bool {
            auto out_string = tokenizer.detokenize({out_token});
            // now prefill done (when ft==0)
            
            // phase clock control
            if (ft == 0 && !_params.fixed_config) {
                freq_config = dvfs.get_cpu_freqs_conf(_params.cpu_clk_idx_d);
                dvfs.set_cpu_freq(freq_config);
                dvfs.set_ram_freq(_params.ram_clk_idx_d);
            }
            // phase pause
            if (ft == 0 && _params.phase_pause > 0) {
                // std::cout << std::flush << "sleep\n"; // test
                this_thread::sleep_for(chrono::milliseconds(_params.phase_pause));
            }

            // generation start
            auto [not_end, output_string] = tokenizer.postprocess(out_string);
            if (!not_end) { return false;} 
            else {
                // std::cout << std::flush << " tp "; // test
                this_thread::sleep_for(chrono::milliseconds(_params.token_pause)); // token pause
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
        
        // Done & store data
        auto sys_time = chrono::duration_cast<chrono::milliseconds>(now_sys_time - start_sys_time).count();
        // proifle_res = { prefill_speed, decode_speed, input_token, output_token, ttft  }
        auto profile_res = model.profiling("Inference");
        profile_res.insert(profile_res.begin(), (double)sys_time/(double)1000.0);
        write_file(profile_res, _params.output_path_infer); // store in real time
        
        // Throttling detection
        // single query is done
        // This throttling detection is valid for only Pixel9
        int cur_cpu_freq = stoi(split_string(execute_cmd(command.c_str()))[0]);
        if (cur_cpu_freq * 1000 != dvfs.get_cpu_freq().at(7).at(freq_config[2])) {
            // deprecated
            // Module::thread_sleep = 0; // reset layer-pause
            _params.phase_pause = 0;          // reset phase-pause
            _params.token_pause = 0;          // reset token-pause
            _params.layer_pause = 0;          // reset layer-pause
                                              // TODO: layer pause must be controlled by _params.
            // new ver.
            model.params.layer_pause = 0;
            throttling = true;     
        }

        // Reset
        if (is_query_save) { ans.push_back(answer); } // accummulate answers
        model.clear_kvcache();
        qa_now++;
        ft = 0;

        // Query-interval
        if((qa_now - qa_start) < qa_limit) this_thread::sleep_for(chrono::milliseconds(_params.query_interval));
    }


    // Hardware Measurement Done
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
    for (int i=0; i<ans.size(); ++i){
        json pair;
        pair["question"] = qa_list[qa_start+i][1];
        pair["answer"] = ans[i];
        qa_array.push_back(pair);
    }

    std::ofstream output_file(output_qa); // open file stream
    if (!output_file) {
        std::cerr << "Error: Could not open file for writing: " << output_qa << std::endl;
        return 1;
    }

    output_file << qa_array.dump(4); // pretty print with indent=4
    output_file.close(); // close file stream
    std::cout << "Saved " << ans.size() << " QA pairs to " << output_qa << "\r\n";

    // termination notification
    // system("input touchscreen keyevent 26");
    // system("input touchscreen keyevent 82");

    return 0;
}