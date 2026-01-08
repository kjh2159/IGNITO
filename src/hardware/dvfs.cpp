#include "dvfs.h"

// DVFS --------------------------------------
const std::map<std::string, std::map<int, std::vector<int>>> DVFS::cpufreq = {
    { "S22_Ultra", {
        { 0, { 307200, 403200, 518400, 614400, 729600, 844800, 960000, 1075200, 1171200, 1267200, 1363200, 1478400, 1574400, 1689600, 1785600 } },
        { 4, { 633600, 768000, 883200, 998400, 1113600, 1209600, 1324800, 1440000, 1555200, 1651200, 1766400, 1881600, 1996800, 2112000, 2227200, 2342400, 2419200 } },
        { 7, { 806400, 940800, 1056000, 1171200, 1286400, 1401600, 1497600, 1612800, 1728000, 1843200, 1958400, 2054400, 2169600, 2284800, 2400000, 2515200, 2630400, 2726400, 2822400, 2841600 } }
    }},
    { "S24", {
        { 0, { 400000, 576000, 672000, 768000, 864000, 960000, 1056000, 1152000, 1248000, 1344000, 1440000, 1536000, 1632000, 1728000, 1824000, 1920000, 1959000 } },
        { 4, { 672000, 768000, 864000, 960000, 1056000, 1152000, 1248000, 1344000, 1440000, 1536000, 1632000, 1728000, 1824000, 1920000, 2016000, 2112000, 2208000, 2304000, 2400000, 2496000, 2592000 } },
        { 7, { 672000, 768000, 864000, 960000, 1056000, 1152000, 1248000, 1344000, 1440000, 1536000, 1632000, 1728000, 1824000, 1920000, 2016000, 2112000, 2208000, 2304000, 2400000, 2496000, 2592000, 2688000, 2784000, 2880000, 2900000 } },
        { 9, { 672000, 768000, 864000, 960000, 1056000, 1152000, 1248000, 1344000, 1440000, 1536000, 1632000, 1728000, 1824000, 1920000, 2016000, 2112000, 2208000, 2304000, 2400000, 2496000, 2592000, 2688000, 2784000, 2880000, 2976000, 3072000, 3207000 } }
    }},
	{ "Fold4", {
		{ 0, { 300000, 441600, 556800, 691200, 806400, 940800, 1056000, 1132800, 1228800, 1324800, 1440000, 1555200, 1670400, 1804800, 1920000, 2016000} },
		{ 4, { 633600, 768000, 883200, 998400, 1113600, 1209600, 1324800, 1440000, 1555200, 1651200, 1766400, 1881600, 1996800, 2112000, 2227200, 2342400, 2457600, 2572800, 2649600, 2745600 } },
		{ 7, { 787200, 921600, 1036800, 1171200, 1286400, 1401600, 1536000, 1651200, 1766400, 1881600, 1996800, 2131200, 2246400, 2361600, 2476800, 2592000, 2707200, 2822400, 2918400, 2995200 } }
	}},
	{ "Pixel9", {
		{ 0, { 820000, 955000, 1098000, 1197000, 1328000, 1425000, 1548000, 1696000, 1849000, 1950000 } },
		{ 4, { 357000, 578000, 648000, 787000, 910000, 1065000, 1221000, 1328000, 1418000, 1549000, 1795000, 1945000, 2130000, 2245000, 2367000, 2450000, 2600000 } },
		{ 7, { 700000, 1164000, 1396000, 1557000, 1745000, 1885000, 1999000, 2147000, 2294000, 2363000, 2499000, 2687000, 2802000, 2914000, 2943000, 2970000, 3015000, 3105000 } }
	}}
};

const std::map<std::string, std::vector<int>> DVFS::ddrfreq = {
    { "S22_Ultra", { 547000, 768000, 1555000, 1708000, 2092000, 2736000, 3196000 } },
    { "S24", { 421000, 676000, 845000, 1014000, 1352000, 1539000, 1716000, 2028000, 2288000, 2730000, 3172000, 3738000, 4206000 } },
    { "Fold4", { 547000, 768000, 1555000, 1708000, 2092000, 2736000, 3196000 } },
    { "Pixel9", { 421000, 546000, 676000, 845000, 1014000, 1352000, 1539000, 1716000, 2028000, 2288000, 2730000, 3172000, 3744000 } }
};


const std::map<std::string, std::vector<std::string>> DVFS::empty_thermal = {
    { "S22_Ultra", { "sdr0-pa0", "sdr1-pa0", "pm8350b_tz", "pm8350b-ibat-lvl0", "pm8350b-ibat-lvl1", "pm8350b-bcl-lvl0", "pm8350b-bcl-lvl1", "pm8350b-bcl-lvl2", "socd", "pmr735b_tz"}},
    { "Fold4", { "sdr0-pa0", "sdr1-pa0", "pm8350b_tz", "pm8350b-ibat-lvl0", "pm8350b-ibat-lvl1", "pm8350b-bcl-lvl0", "pm8350b-bcl-lvl1", "pm8350b-bcl-lvl2", "socd", "pmr735b_tz", "qcom,secure-non"}},
    { "S24", {}},
    { "Pixel9", {}}
};


// consturctor
DVFS::DVFS(const std::string& device_name) : Device(device_name) { output_filename = ""; }
DVFS::~DVFS() { close_fd_cache(); }


const std::map<int, std::vector<int>>& DVFS::get_cpu_freq() const {
    return cpufreq.at(device);
}
const std::vector<std::string>& DVFS::get_empty_thermal() const {
    return empty_thermal.at(device);
}

const std::vector<int>& DVFS::get_ddr_freq() const {
    return ddrfreq.at(device);
}

std::vector<int> DVFS::get_cpu_freqs_conf(int prime_cpu_index){
    int prime_cluster_id = this->cluster_indices[this->cluster_indices.size()-1];
    int max_prime_cluster_idx = this->get_cpu_freq().at(prime_cluster_id).size()-1;
    
    // integrity check
    if (prime_cpu_index > max_prime_cluster_idx ){
        std::cerr << "[WARNING] Too big prime_cpu_index: " << prime_cpu_index << " > " << max_prime_cluster_idx << std::endl;
    }


    // generate frequency configuration
    std::vector<int> freq_conf = {};
    for (auto cluster_idx : this->cluster_indices){
        int max_idx = this->get_cpu_freq().at(cluster_idx).size()-1;
        int idx = static_cast<int>(
            std::round(((double)prime_cpu_index/(double)max_prime_cluster_idx)*(double)max_idx)
        );

        freq_conf.push_back(idx);
    }

    return freq_conf;
}
// -------------------------------------------


// Collector ----------------------------------
Collector::Collector(const std::string& device_name) : Device(device_name) {}

// pixel9
// BIG: thermal/thermal_zone0
// MID: thermal/thermal_zone1
const std::map<std::string, std::vector<std::string>> Collector::thermal_zones_cpu = {
    { "Pixel9", { /*BIG*/ "/sys/devices/virtual/thermal/thermal_zone0", /*MID*/ "/sys/devices/virtual/thermal/thermal_zone1" } }
};

double Collector::collect_high_temp(){
    if (this->device != "Pixel9") return 0.0;

    std::string command = "su -c \"";
    for (auto zone_path : this->thermal_zones_cpu.at(this->device)){
        command += std::string("awk '{print \\$1/1000}' ")+zone_path+std::string("/temp; ");
    }
    command += "\""; // closing quote

    std::string output = execute_cmd(command.c_str());
    std::vector<std::string> temps = split_string(output);

    // print high temperature
    std::vector<double> temp_vals = {};
    for (auto t_str : temps){
        temp_vals.push_back(std::stod(t_str));
    }

    return std::max_element(temp_vals.begin(), temp_vals.end())[0];
}

// -------------------------------------------


int DVFS::open_wr(const std::string& path) {
    // open with O_CLOEXEC to prevent FD leak to child processes
    int fd = open(path.c_str(), O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "[DVFS] open failed: %s (%s)\n", path.c_str(), strerror(errno));
    }
    return fd;
}

void DVFS::close_fd(int& fd) {
    // close fd
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

bool DVFS::try_open_first(const std::vector<std::string>& candidates, int& out_fd) {
    // try open files in candidates sequentially
    for (const auto& p : candidates) {
        int fd = open_wr(p);
        if (fd >= 0) {
            out_fd = fd;
            return true;
        }
    }
    out_fd = -1;
    return false;
}

int DVFS::write_fd_int(int fd, long long v) {
    // write integer status to fd
    
    if (fd < 0) return -1;

    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%lld\n", v);
    if (len <= 0) return -2;

    // sysfs: offset 0 write is safe
    (void)lseek(fd, 0, SEEK_SET);

    const char* p = buf;
    int left = len;
    while (left > 0) {
        ssize_t n = write(fd, p, left);
        if (n < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "[DVFS] write failed (fd=%d): %s\n", fd, strerror(errno));
            return -3;
        }
        p += n;
        left -= (int)n;
    }
    return 0;
}

// 1) FD cache initialization
int DVFS::init_fd_cache() {
    std::lock_guard<std::mutex> lk(io_mu);

    close_fd_cache_nolock(); // if already opened, close first

    // CPU policy fds
    cpu_fds.clear();
    cpu_fds.reserve(cluster_indices.size());

    for (int idx : cluster_indices) {
        CpuPolicyFD p;
        p.policy_idx = idx;

        //  Pixel9 and S24 have same path structure
        const std::string base = "/sys/devices/system/cpu/cpufreq/policy" + std::to_string(idx);
        p.max_fd = open_wr(base + "/scaling_max_freq");
        p.min_fd = open_wr(base + "/scaling_min_freq");

        if (p.max_fd < 0 || p.min_fd < 0) {
            fprintf(stderr, "[DVFS] policy%d open incomplete (need root?)\n", idx);
            close_fd(p.max_fd);
            close_fd(p.min_fd);
            // if failure, close all and return error
            close_fd_cache();
            fd_ready = false;
            return -1;
        }

        cpu_fds.push_back(p);
    }

    // MIF(devfreq) fds (RAM)
    // Pixel 9 and S24 have same base path
    mif_fds.base = "/sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif";
    {
        // Depending on device and kernel, the min/max path differs
        std::vector<std::string> min_candidates = {
            mif_fds.base + "/scaling_devfreq_min", // S24
            mif_fds.base + "/min_freq", // Pixel9
            mif_fds.base + "/scaling_min_freq"
        };

        std::vector<std::string> max_candidates;
        if (get_device_name() == "Pixel9") {
            max_candidates = {
                mif_fds.base + "/max_freq", // Pixel9 preferred
                mif_fds.base + "/scaling_devfreq_max"
            };
        } else {
            max_candidates = {
                mif_fds.base + "/scaling_devfreq_max", // S24 preferred
                mif_fds.base + "/max_freq"
            };
        }

        if (!try_open_first(min_candidates, mif_fds.min_fd)) {
            fprintf(stderr, "[DVFS] MIF min open failed (need root? path mismatch)\n");
            close_fd_cache();
            fd_ready = false;
            return -2;
        }
        if (!try_open_first(max_candidates, mif_fds.max_fd)) {
            fprintf(stderr, "[DVFS] MIF max open failed (need root? path mismatch)\n");
            close_fd_cache();
            fd_ready = false;
            return -3;
        }
    }

    fd_ready = true;
    return 0;
}

// 2) FD cache cleanup
void DVFS::close_fd_cache() {
    std::lock_guard<std::mutex> lk(io_mu);
    close_fd_cache_nolock(); 
}

void DVFS::close_fd_cache_nolock() {
    // close all cached fds with no lock
    // assume io_mu is already locked
    // to avoid deadlock
    for (auto& p : cpu_fds) {
        close_fd(p.max_fd);
        close_fd(p.min_fd);
    }
    cpu_fds.clear();

    close_fd(mif_fds.min_fd);
    close_fd(mif_fds.max_fd);

    fd_ready = false;
}

// 3) set/unset: directly write if FD cache is ready
int DVFS::set_cpu_freq(const std::vector<int>& freq_indices) {
    if ((int)cluster_indices.size() != (int)freq_indices.size()) return 1;

    std::lock_guard<std::mutex> lk(io_mu);

    if (!fd_ready) {
        fprintf(stderr, "[DVFS] fd cache not ready. call init_fd_cache() first.\n");
        return 2;
    }

    // max first, min last (protect min > max being set)
    for (int i = 0; i < (int)cluster_indices.size(); ++i) {
        int policy = cluster_indices[i];
        int freq_idx = freq_indices[i];

        const auto& table = cpufreq.at(device).at(policy);
        if (freq_idx < 0 || freq_idx >= (int)table.size()) return 3;

        int clk = table[freq_idx];

        // search corresponding policy fd (if sequence is identical, cpu_fds[i] can be used directly)
        // safe policy match
        CpuPolicyFD* fdp = nullptr;
        for (auto& p : cpu_fds) if (p.policy_idx == policy) { fdp = &p; break; }
        if (!fdp) return 4;

        if (write_fd_int(fdp->max_fd, clk) != 0) return 5;
        if (write_fd_int(fdp->min_fd, clk) != 0) return 6;
    }
    return 0;
}

int DVFS::unset_cpu_freq() {
    // unset to default (min: lowest, max: highest)

    std::lock_guard<std::mutex> lk(io_mu);

    if (!fd_ready) {
        fprintf(stderr, "[DVFS] fd cache not ready. call init_fd_cache() first.\n");
        return 2;
    }

    for (int policy : cluster_indices) {
        const auto& table = cpufreq.at(device).at(policy);
        int min_clk = table.front();
        int max_clk = table.back();

        CpuPolicyFD* fdp = nullptr;
        for (auto& p : cpu_fds) if (p.policy_idx == policy) { fdp = &p; break; }
        if (!fdp) return 4;

        if (write_fd_int(fdp->max_fd, max_clk) != 0) return 5;
        if (write_fd_int(fdp->min_fd, min_clk) != 0) return 6;
    }
    return 0;
}

int DVFS::set_ram_freq(const int freq_idx) {
    std::lock_guard<std::mutex> lk(io_mu);

    if (!fd_ready) {
        fprintf(stderr, "[DVFS] fd cache not ready. call init_fd_cache() first.\n");
        return 2;
    }

    const auto& table = get_ddr_freq();
    if (freq_idx < 0 || freq_idx >= (int)table.size()) return 1;

    int clk = table[freq_idx];

    // max first, min last (policy-dependent, but this form is generally safe)
    if (write_fd_int(mif_fds.max_fd, clk) != 0) return 3;
    if (write_fd_int(mif_fds.min_fd, clk) != 0) return 4;
    return 0;
}

int DVFS::unset_ram_freq() {
    std::lock_guard<std::mutex> lk(io_mu);

    if (!fd_ready) {
        fprintf(stderr, "[DVFS] fd cache not ready. call init_fd_cache() first.\n");
        return 2;
    }

    const auto& table = get_ddr_freq();
    int min_clk = table.front();
    int max_clk = table.back();

    if (write_fd_int(mif_fds.max_fd, max_clk) != 0) return 3;
    if (write_fd_int(mif_fds.min_fd, min_clk) != 0) return 4;
    return 0;
}