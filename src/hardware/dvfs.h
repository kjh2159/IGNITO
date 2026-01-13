#ifndef DVFS_H
#define DVFS_H

#include "device.h"
#include "utils.h"

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <map>
#include <cmath>
#include <algorithm>
#include <fstream>
#include <cstdio>
#include <sstream>
#include <mutex>

class Collector;

class Collector : public Device {
private:
    // pixel9
    // BIG: thermal/thermal_zone0
    // MID: thermal/thermal_zone1
    static const std::map<std::string, std::vector<std::string>> thermal_zones_cpu;
public:
    explicit Collector(const std::string& device_name);
    double collect_high_temp();

};


/* ** Example of DVFS class **

DVFS dvfs("Pixel9");
if (dvfs.init_fd_cache() != 0) {
    fprintf(stderr, "FD cache initialization failed. Are you root or authorized?\n");
}

... (skip) ...

std::vector<int> freq_config = dvfs.get_cpu_freqs_conf(prime_cpu_index);
dvfs.set_cpu_freq(freq_config);
dvfs.set_ram_freq(ram_freq_index);
... (skip) ...
dvfs.unset_cpu_freq();
dvfs.unset_ram_freq();

*/
class DVFS : public Device {
private:
    static const std::map<std::string, std::map<int, std::vector<int>>> cpufreq;
    static const std::map<std::string, std::vector<int>> ddrfreq;
    static const std::map<std::string, std::vector<std::string>> empty_thermal;

private:
    // ---- FD cache structure ----
    struct CpuPolicyFD {
        int policy_idx = -1;
        int min_fd = -1; // scaling_min_freq
        int max_fd = -1; // scaling_max_freq
    };

    struct MifFD {
        int min_fd = -1; // scaling_devfreq_min (or min_freq)
        int max_fd = -1; // scaling_devfreq_max (or max_freq)
        std::string base;
    };

    std::vector<CpuPolicyFD> cpu_fds;
    MifFD mif_fds;
    bool fd_ready = false;
    std::mutex io_mu; // mutex lock guard for fd cache I/O

public:
    std::string output_filename;

public:
    DVFS(const std::string& device_name);
    ~DVFS();

    const std::map<int, std::vector<int>>& get_cpu_freq() const;
    const std::vector<int>& get_ddr_freq() const;
	const std::vector<std::string>& get_empty_thermal() const;

	int set_cpu_freq(const std::vector<int>&);
    int unset_cpu_freq();
    int set_ram_freq(const int freq_idx);
    int unset_ram_freq();

    std::vector<int> get_cpu_freqs_conf(int prime_cpu_index);

    Collector get_collector() { return Collector(this->get_device_name()); }

    // FD cache
    int init_fd_cache();    // sysfs open
    void close_fd_cache();  // sysfs close
    bool fd_cache_enabled() const { return fd_ready; }

private:
    // internal helper
    static int open_wr(const std::string& path);
    static int write_fd_int(int fd, long long v);
    static void close_fd(int& fd);
    static bool try_open_first(const std::vector<std::string>& candidates, int& out_fd);
    void close_fd_cache_nolock();
};

#endif //DVFS_H