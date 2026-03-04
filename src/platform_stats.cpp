// Linux implementation: read per-process CPU/time and memory from /proc and
// try to detect per-process GPU memory via nvidia-smi.

#include "platform_stats.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/types.h>

// Helper: read total jiffies from /proc/stat (sum of all cpu fields)
static bool read_total_jiffies(unsigned long long &total) {
    std::ifstream f("/proc/stat");
    if(!f.is_open()) return false;
    std::string line;
    if(!std::getline(f, line)) return false;
    std::istringstream ss(line);
    std::string cpu;
    ss >> cpu; // "cpu"
    unsigned long long v;
    total = 0;
    while(ss >> v) total += v;
    return true;
}

// Read process jiffies (utime + stime) from /proc/<pid>/stat
// Read process jiffies (utime + stime) from /proc/<pid>/stat
static bool read_proc_jiffies(pid_t pid, unsigned long long &proc_jiffies) {
    std::string path = std::string("/proc/") + std::to_string(pid) + "/stat";
    std::ifstream f(path);
    if(!f.is_open()) return false;
    std::string content;
    std::getline(f, content);
    
    // skip comm (may contain spaces) which is between first '(' and last ')'
    size_t rparen = content.rfind(')');
    if(rparen == std::string::npos) return false;
    std::string rest = content.substr(rparen+2); // skip ") "
    std::istringstream ss(rest);
    
    // PERBAIKAN: Gunakan std::string untuk skip agar aman saat membaca huruf (state 'S'/'R')
    std::string skip; 
    for(int i=0; i<11; i++) {
        ss >> skip;
    }
    
    unsigned long long utime=0, stime=0;
    ss >> utime >> stime;
    proc_jiffies = utime + stime;
    return true;
}

// Read process memory (VmRSS and VmSize) from /proc/<pid>/status
static bool read_proc_mem_kb(pid_t pid, long &rss_kb, long &vms_kb) {
    std::string path = std::string("/proc/") + std::to_string(pid) + "/status";
    std::ifstream f(path);
    if(!f.is_open()) return false;
    std::string line;
    rss_kb = vms_kb = 0;
    while(std::getline(f, line)){
        if(line.rfind("VmRSS:", 0) == 0){
            std::istringstream ss(line);
            std::string key; ss >> key >> rss_kb;
        } else if(line.rfind("VmSize:", 0) == 0){
            std::istringstream ss(line);
            std::string key; ss >> key >> vms_kb;
        }
        if(rss_kb && vms_kb) break;
    }
    return (rss_kb!=0 || vms_kb!=0);
}

// Estimate per-process GPU utilization percent by attributing device-wide
// utilization proportionally to per-process GPU memory usage (heuristic).
// Returns estimated percent for given pid, or -1 on failure/not available.
double read_gpu_util_for_pid(pid_t pid){
    // First, read per-process GPU memory usage via nvidia-smi (pid, used_memory MiB)
    FILE *p = popen("nvidia-smi --query-compute-apps=pid,used_memory --format=csv,noheader,nounits 2>/dev/null","r");
    if(!p) return -1;
    char buf[256];
    unsigned long long total_mem = 0;
    unsigned long long my_mem = 0;
    while(fgets(buf, sizeof(buf), p)){
        int line_pid = 0; unsigned long long mem = 0;
        if(sscanf(buf, "%d, %llu", &line_pid, &mem) == 2){
            total_mem += mem;
            if((pid_t)line_pid == pid) my_mem = mem;
        }
    }
    pclose(p);

    if(total_mem == 0) {
        // no process GPU memory found; cannot attribute
        return -1;
    }

    // Read device-wide GPU utilization (first GPU) as percent
    FILE *q = popen("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null","r");
    if(!q) return -1;
    double device_util = -1;
    if(fgets(buf, sizeof(buf), q)){
        int v = 0;
        if(sscanf(buf, "%d", &v) == 1) device_util = (double)v;
    }
    pclose(q);

    if(device_util < 0) return -1;

    // Estimate per-process utilization proportional to memory usage
    double frac = (double)my_mem / (double)total_mem;
    double est = device_util * frac;
    return est;
}

bool read_process_stats(SysStats &s){
    pid_t pid = getpid();
    static unsigned long long prev_total = 0;
    static unsigned long long prev_proc = 0;
    static bool inited = false;

    unsigned long long total_jiffies = 0;
    if(!read_total_jiffies(total_jiffies)) return false;
    unsigned long long proc_jiffies = 0;
    if(!read_proc_jiffies(pid, proc_jiffies)) return false;

    double cpu_percent = 0.0;
    if(!inited){
        // store and return zero for first sample
        prev_total = total_jiffies;
        prev_proc = proc_jiffies;
        inited = true;
        cpu_percent = 0.0;
    } else {
        unsigned long long delta_total = total_jiffies - prev_total;
        unsigned long long delta_proc = proc_jiffies - prev_proc;
        if(delta_total > 0) cpu_percent = (double)delta_proc * 100.0 / (double)delta_total;
        else cpu_percent = 0.0;
        prev_total = total_jiffies;
        prev_proc = proc_jiffies;
    }

    long rss_kb=0, vms_kb=0;
    read_proc_mem_kb(pid, rss_kb, vms_kb);

    double gpu_util = read_gpu_util_for_pid(pid);

    s.proc_cpu_percent = cpu_percent;
    s.proc_rss_kb = rss_kb;
    s.proc_vms_kb = vms_kb;
    s.gpu_util_percent = gpu_util; // -1 if not available
    return true;
}
