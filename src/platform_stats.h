// Simple cross-platform helpers to query CPU, memory and GPU usage.
// Currently implemented for Linux (/proc) and optional NVIDIA GPU via nvidia-smi.

#pragma once

#include <string>

struct SysStats {
    double proc_cpu_percent;   // CPU% used by this process (0..100)
    long proc_rss_kb;          // Resident Set Size (KB) for this process
    long proc_vms_kb;          // Virtual memory size (KB) for this process
    double gpu_util_percent;   // GPU utilization percent (device-wide), -1 if N/A
};

// Read current process stats (CPU% since last call, memory). The PID used is the current process.
// Returns true on success. The first call may return cpu=0 as a baseline.
bool read_process_stats(SysStats &s);

// Estimate per-process GPU utilization percent using nvidia-smi by attributing
// device-wide utilization proportionally to per-process GPU memory usage.
// Returns estimated percent for the given pid, or -1 on failure/not-found.
double read_gpu_util_for_pid(pid_t pid);
