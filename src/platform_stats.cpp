// Linux implementation: read per-process CPU/time and memory from /proc and
// try to detect GPU usage via nvidia-smi. For OpenGL graphics workloads,
// per-process GPU utilization is best-effort only.

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

    unsigned long long v = 0;
    total = 0;
    while(ss >> v) total += v;
    return true;
}

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

    std::string rest = content.substr(rparen + 2); // skip ") "
    std::istringstream ss(rest);

    // Skip fields after state until utime/stime
    std::string skip;
    for(int i = 0; i < 11; i++) {
        ss >> skip;
    }

    unsigned long long utime = 0, stime = 0;
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

    while(std::getline(f, line)) {
        if(line.rfind("VmRSS:", 0) == 0) {
            std::istringstream ss(line);
            std::string key;
            ss >> key >> rss_kb;
        } else if(line.rfind("VmSize:", 0) == 0) {
            std::istringstream ss(line);
            std::string key;
            ss >> key >> vms_kb;
        }

        if(rss_kb && vms_kb) break;
    }

    return (rss_kb != 0 || vms_kb != 0);
}

static double parse_metric_token(const std::string &s) {
    if(s.empty() || s == "-" || s == "N/A") return -1.0;
    char *end = nullptr;
    double v = std::strtod(s.c_str(), &end);
    if(end == s.c_str()) return -1.0;
    return v;
}

double read_gpu_util_for_pid(pid_t pid) {
#ifdef TARGET_RENESAS
    return -1.0;
#else
    char buf[512];

    // =========================================================
    // 1) Coba per-process monitoring via nvidia-smi pmon
    //    Lebih cocok untuk graphics/OpenGL dibanding compute-apps
    // =========================================================
    FILE *p = popen("nvidia-smi pmon -c 1 2>/dev/null", "r");
    if(p) {
        double found_util = -1.0;

        while(fgets(buf, sizeof(buf), p)) {
            if(buf[0] == '#') continue;

            std::istringstream ss(buf);
            std::string gpu_s, pid_s, type_s, sm_s, mem_s, enc_s, dec_s;

            ss >> gpu_s >> pid_s >> type_s >> sm_s >> mem_s >> enc_s >> dec_s;
            if(!ss) continue;

            if(pid_s == "-" || pid_s.empty()) continue;

            int line_pid = std::atoi(pid_s.c_str());
            if(line_pid == (int)pid) {
                double sm  = parse_metric_token(sm_s);
                double mem = parse_metric_token(mem_s);
                double enc = parse_metric_token(enc_s);
                double dec = parse_metric_token(dec_s);

                double util = -1.0;
                if(sm  >= 0.0) util = std::max(util, sm);
                if(mem >= 0.0) util = std::max(util, mem);
                if(enc >= 0.0) util = std::max(util, enc);
                if(dec >= 0.0) util = std::max(util, dec);

                found_util = util;
                break;
            }
        }

        pclose(p);

        if(found_util >= 0.0) {
            return found_util;
        }
    }

    // =========================================================
    // 2) Fallback lama: query-compute-apps
    //    Cocok untuk CUDA compute apps, sering gagal untuk OpenGL
    // =========================================================
    FILE *q = popen("nvidia-smi --query-compute-apps=pid,used_memory --format=csv,noheader,nounits 2>/dev/null", "r");
    if(q) {
        unsigned long long total_mem = 0;
        unsigned long long my_mem = 0;

        while(fgets(buf, sizeof(buf), q)) {
            int line_pid = 0;
            unsigned long long mem = 0;
            if(std::sscanf(buf, "%d, %llu", &line_pid, &mem) == 2) {
                total_mem += mem;
                if((pid_t)line_pid == pid) my_mem = mem;
            }
        }
        pclose(q);

        if(total_mem > 0) {
            FILE *r = popen("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null", "r");
            if(r) {
                double device_util = -1.0;
                if(fgets(buf, sizeof(buf), r)) {
                    int v = 0;
                    if(std::sscanf(buf, "%d", &v) == 1) {
                        device_util = (double)v;
                    }
                }
                pclose(r);

                if(device_util >= 0.0) {
                    double frac = (double)my_mem / (double)total_mem;
                    return device_util * frac;
                }
            }
        }
    }

    // =========================================================
    // 3) Fallback terakhir: util GPU total device
    // =========================================================
    FILE *s = popen("nvidia-smi --query-gpu=utilization.gpu --format=csv,noheader,nounits 2>/dev/null", "r");
    if(s) {
        double device_util = -1.0;
        if(fgets(buf, sizeof(buf), s)) {
            int v = 0;
            if(std::sscanf(buf, "%d", &v) == 1) {
                device_util = (double)v;
            }
        }
        pclose(s);

        if(device_util >= 0.0) return device_util;
    }

    return -1.0;
#endif
}

bool read_process_stats(SysStats &s) {
    pid_t pid = getpid();

    static unsigned long long prev_total = 0;
    static unsigned long long prev_proc = 0;
    static bool inited = false;

    unsigned long long total_jiffies = 0;
    if(!read_total_jiffies(total_jiffies)) return false;

    unsigned long long proc_jiffies = 0;
    if(!read_proc_jiffies(pid, proc_jiffies)) return false;

    double cpu_percent = 0.0;
    if(!inited) {
        prev_total = total_jiffies;
        prev_proc = proc_jiffies;
        inited = true;
        cpu_percent = 0.0;
    } else {
        unsigned long long delta_total = total_jiffies - prev_total;
        unsigned long long delta_proc  = proc_jiffies - prev_proc;

        if(delta_total > 0) {
            cpu_percent = (double)delta_proc * 100.0 / (double)delta_total;
        } else {
            cpu_percent = 0.0;
        }

        prev_total = total_jiffies;
        prev_proc  = proc_jiffies;
    }

    long rss_kb = 0, vms_kb = 0;
    read_proc_mem_kb(pid, rss_kb, vms_kb);

    double gpu_util = read_gpu_util_for_pid(pid);

    s.proc_cpu_percent = cpu_percent;
    s.proc_rss_kb = rss_kb;
    s.proc_vms_kb = vms_kb;
    s.gpu_util_percent = gpu_util;

    return true;
}