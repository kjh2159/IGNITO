#include "record.h"
#include <algorithm>

/*
 * Temp test function 
 */
static pid_t gettid_() { return (pid_t)syscall(SYS_gettid); }

static void pin_tid(pid_t tid, std::initializer_list<int> cpus) {
  cpu_set_t cs; CPU_ZERO(&cs);
  for (int c : cpus) CPU_SET(c, &cs);
  sched_setaffinity(tid, sizeof(cs), &cs);
}
static void pin_current(std::initializer_list<int> cpus) {
  pin_tid(gettid_(), cpus);
}
/*
* Temp test function
*/


// test function
void get_cpu_info() {
    std::string command = "su -c \""; //prefix
    
    // command to get cpu freq
    command += "awk '{print \\$1/1000}' /sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq; ";
    command += "awk '{print \\$1/1000}' /sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq; ";
    command += "\""; // postfix

    // only execution
    system(command.c_str());
    //std::string output = execute_cmd(command.c_str());
    //std::cout << execute_cmd(command.c_str())[0] << std::endl;
}



const std::string get_records_names(const DVFS& dvfs) {
    std::string names = "Time,";
    
    // thermal info
    std::string command = "su -c \"cat /sys/devices/virtual/thermal/thermal_zone*/type\"";
    std::string temp_record = execute_cmd(command.c_str());
    std::replace(temp_record.begin(), temp_record.end(), '\n', ',');
    names += temp_record;

    // gpu info
    names += "gpu_min_clock,gpu_max_clock,";

    // cpu info
    for(const auto index : dvfs.get_cluster_indices()){
        names += std::string("cpu") + std::to_string(index) + std::string("_max_freq,cpu") + std::to_string(index) + "_cur_freq,";
    }

    // mem info
    command = "su -c \"awk '{print \\$1}' /proc/meminfo\"";
    temp_record = execute_cmd(command.c_str());
    std::replace(temp_record.begin(), temp_record.end(), ':', '\0');
    std::replace(temp_record.begin(), temp_record.end(), '\n', ',');
    names += temp_record;

    // power
    if (dvfs.get_device_name() == "Pixel9") names += "current_now,voltage_now,";
    else names += "power_now,current_now,voltage_now,";

    // RAM clock info
    names += "scaling_devfreq_max,scaling_devfreq_min,cur_freq,";

    #if defined(MV_COLLECT)
        names += "cpu_mid_power,cpu_mid_current,cpu_big_power,cpu_big_current,cpu_mid_M_power,cpu_mid_M_current,";
    #endif

    // remove emptyThermal 
	for (std::string empty : dvfs.get_empty_thermal()){
		if (empty == "qcom,secure-non"){
			std::size_t p = 0;
			while( (p = names.find(empty, p)) != std::string::npos ){
				names.replace(p, empty.length(), "secure-non");
			}
		    continue;
		}
		std::string temp = empty + ",";
		std::size_t pos = 0;
		while( (pos = names.find(temp, pos)) != std::string::npos){
			names.replace(pos, temp.length(), ""); // string replace
		}
	}

	return names;
}


/* 
 * GET HARD RECORDS function
 * - args 
 *   	- cluster_indices: an integer vector or array to contain cluster indices (ex. {0,4,7})
 * - task
 *   	- Get hard records such as thermal, power, etc.
 * - return
 *   	- A string vector to contain outputs
 * */
std::vector<std::string> get_hard_records(const DVFS& dvfs) {
    std::vector<int> cluster_indices = dvfs.get_cluster_indices();
    std::string device_name = dvfs.get_device_name();

    std::string command = "su -c \""; // prefix
    
    // thermal info
    command += "awk '{print \\$1/1000}' /sys/devices/virtual/thermal/thermal_zone*/temp; ";
	      
	// GPU clock info
    if (device_name == "Pixel9"){
        command += "awk '{print \\$1}' /sys/devices/platform/1f000000.mali/scaling_min_freq; awk '{print \\$1}' /sys/devices/platform/1f000000.mali/scaling_max_freq; "; //gpu clock
    } else { // S24
	    command += "awk '{print \\$1}' /sys/kernel/gpu/gpu_min_clock; awk '{print \\$1}' /sys/kernel/gpu/gpu_max_clock; ";
    }

    // CPU clock info
    for (std::size_t i=0; i<cluster_indices.size(); ++i){
        int idx = cluster_indices[i];
        command += std::string("awk '{print \\$1/1000}' /sys/devices/system/cpu/cpu")+std::to_string(idx)+std::string("/cpufreq/scaling_max_freq; ") +
		   std::string("awk '{print \\$1/1000}' /sys/devices/system/cpu/cpu")+std::to_string(idx)+std::string("/cpufreq/scaling_cur_freq; ");
    }

	// RAM info
    command += "awk '{print \\$2/1024}' /proc/meminfo; ";

    // Power Consumption info
    if (device_name == "Pixel9"){
        command += "awk '{print}' /sys/class/power_supply/battery/current_now; ";
        command += "awk '{print}' /sys/class/power_supply/battery/voltage_now; ";
    } else {
        command += "awk '{print}' /sys/class/power_supply/battery/power_now; "; // pixel does not contain
        command += "awk '{print}' /sys/class/power_supply/battery/current_now; ";
        command += "awk '{print}' /sys/class/power_supply/battery/voltage_now; ";
    }

    // RAM clock info (S24/Pixel9)
    if (device_name == "Pixel9"){
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/max_freq; ";
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/scaling_devfreq_min; ";
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/cur_freq; ";
    } else { // S24
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/scaling_devfreq_max; ";
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/scaling_devfreq_min; ";
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/cur_freq; ";
    }

    #if defined(MV_COLLECT)
        command += "awk '{print \\$1}' /sys/bus/iio/devices/iio:device1/in_power2_scale; awk '{print \\$1}' /sys/bus/iio/devices/iio:device1/in_current2_scale; "; //CPU (MID)
        command += "awk '{print \\$1}' /sys/bus/iio/devices/iio:device1/in_power3_scale; awk '{print \\$1}' /sys/bus/iio/devices/iio:device1/in_current3_scale; "; //CPU (BIG)
        command += "awk '{print \\$1}' /sys/bus/iio/devices/iio:device1/in_power8_scale; awk '{print \\$1}' /sys/bus/iio/devices/iio:device1/in_current8_scale; "; //CPU (MID)
    #endif

    // closing quote
    command += "\"";
    // execute command and get output
    std::string output = execute_cmd(command.c_str());
    

	//std::cout << command << std::endl; // test
    // string post-processing
    return split_string(output);
}


std::vector<std::string> get_hard_records_wo_systime(const DVFS& dvfs){
    std::vector<int> cluster_indices = dvfs.get_cluster_indices();
    std::string device_name = dvfs.get_device_name();

    std::string command = "su -c \""; // prefix
    
    // thermal info
    command += "awk '{print \\$1/1000}' /sys/devices/virtual/thermal/thermal_zone*/temp; ";
	      
	// GPU clock info
    if (device_name == "Pixel9"){
        command += "awk '{print \\$1}' /sys/devices/platform/1f000000.mali/scaling_min_freq; awk '{print \\$1}' /sys/devices/platform/1f000000.mali/scaling_max_freq; "; //gpu clock
    } else { // S24
	    command += "awk '{print \\$1}' /sys/kernel/gpu/gpu_min_clock; awk '{print \\$1}' /sys/kernel/gpu/gpu_max_clock; ";
    }

    // CPU clock info
    for (std::size_t i=0; i<cluster_indices.size(); ++i){
        int idx = cluster_indices[i];
        command += std::string("awk '{print \\$1/1000}' /sys/devices/system/cpu/cpu")+std::to_string(idx)+std::string("/cpufreq/scaling_max_freq; ") +
		   std::string("awk '{print \\$1/1000}' /sys/devices/system/cpu/cpu")+std::to_string(idx)+std::string("/cpufreq/scaling_cur_freq; ");
    }

	// RAM info
    command += "awk '{print \\$2/1024}' /proc/meminfo; ";

    // Power Consumption info
    if (device_name == "Pixel9"){
        command += "awk '{print}' /sys/class/power_supply/battery/current_now; ";
        command += "awk '{print}' /sys/class/power_supply/battery/voltage_now; ";
    } else {
        command += "awk '{print}' /sys/class/power_supply/battery/power_now; "; // pixel does not contain
        command += "awk '{print}' /sys/class/power_supply/battery/current_now; ";
        command += "awk '{print}' /sys/class/power_supply/battery/voltage_now; ";
    }

    // RAM clock info (S24/Pixel9)
    if (device_name == "Pixel9"){
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/max_freq; ";
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/scaling_devfreq_min; ";
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/cur_freq; ";
    } else { // S24
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/scaling_devfreq_max; ";
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/scaling_devfreq_min; ";
        command += "awk '{print \\$1/1000}' /sys/devices/platform/17000010.devfreq_mif/devfreq/17000010.devfreq_mif/cur_freq; ";
    }


    // closing quote
    command += "\"";
    // execute command and get output
    std::string output = execute_cmd(command.c_str());
    

	//std::cout << command << std::endl; // test
    // string post-processing
    return split_string(output);
}


void write_file(const std::vector<std::string>& data, std::string output){
    
    // open file append mode
	std::ofstream file(output, std::ios::app);

	// check file open
	if (!file){
		std::cerr << "failed to open file: " << HARD_RECORD_FILE << std::endl;
		return;
	}

	// wrtie file
	for (const auto v : data){
		file << v << ",";
	}
	file << "\n";

	// close file
	file.close();
}

void write_file(const std::string& data, std::string output){
	// open file append mode
	std::ofstream file(output, std::ios::app);

	// check file open
	if (!file){
		std::cerr << "failed to open file: " << HARD_RECORD_FILE << std::endl;
		return;
	}

	// wrtie file
	file << data << "\n";

	// close file
	file.close();
}

void write_file(const std::vector<double>& data, std::string output){
    
    // open file append mode
	std::ofstream file(output, std::ios::app);

	// check file open
	if (!file){
		std::cerr << "failed to open file: " << HARD_RECORD_FILE << std::endl;
		return;
	}

	// wrtie file
	for (const auto v : data){
		file << v << ",";
	}
	file << "\n";

	// close file
	file.close();
}

/*
 * TO DO
 * ### This function should be called by background process! ###
 * 
 * ### sigterm should be true after experiment completion
 * 
 * */
void record_hard(std::atomic<bool>& sigterm, const DVFS& dvfs){
    pin_current({2}); // silver core for agent
    sigterm = false;
    std::string filename = dvfs.output_filename;


	// insert hard names
	write_file(get_records_names(dvfs), filename);

	
	int test_index = 0;
	std::vector<std::string> records;
    auto start_sys_time = std::chrono::system_clock::now();
    do{
        // get records
		records = get_hard_records(dvfs);
        auto now = std::chrono::system_clock::now();
		auto sys_time = std::chrono::duration_cast<std::chrono::milliseconds>(now-start_sys_time).count(); // ms base
		records.insert(records.begin(), std::to_string((double)sys_time/(double)1000)); // insert systime into firstrecord element
		
		// File write record
		write_file(records, filename);
        // wait
        //std::this_thread::sleep_for(std::chrono::milliseconds(170));

// tester code: start
//		test_index++;
//		if (test_index == 3) sigterm = true;
// tester code: end

    }while(sigterm != true);
}
