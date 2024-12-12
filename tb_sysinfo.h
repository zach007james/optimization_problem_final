//
// Dynamic System Core / Thread Info Helper
// (Ⓒ Zachary R. James)
// (@tinybloom).
//


#ifndef TB_SYSINFO_H
#define TB_SYSINFO_H

/* FUNCTIONALITY */
// print_sys_info() - prints the scores, threads, and threads per core
// detect_physical_cores() - detects the number of physical cores
// detect_logical_cores() - detects the number of logical cores
// detect_threads_per_core() - detects the number of threads per core
/* FUNCTIONALITY */

#include <stdio.h>
#include <string.h>

/* Detecting Cores and Threads */
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>

int detect_physical_cores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

int detect_logical_cores() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

#elif defined(__linux__)
#include <unistd.h>

int detect_physical_cores() {
    FILE* fp = fopen("/proc/cpuinfo", "r");
    if (fp) {
        char line[256];
        int physical_cores = 0;
        while (fgets(line, sizeof(line), fp)) {
            if (strncmp(line, "cpu cores", 9) == 0) {
                sscanf(line, "cpu cores : %d", &physical_cores);
                break;
            }
        }
        fclose(fp);
        return physical_cores > 0 ? physical_cores : sysconf(_SC_NPROCESSORS_ONLN);
    }
    return sysconf(_SC_NPROCESSORS_ONLN);
}

int detect_logical_cores() {
    long nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    return (nprocs > 0) ? nprocs : 1;
}

#elif defined(__APPLE__)
#include <unistd.h>
#include <sys/sysctl.h>

int detect_physical_cores() {
    int physical_cores;
    size_t len = sizeof(physical_cores);
    sysctlbyname("hw.physicalcpu", &physical_cores, &len, NULL, 0);
    return physical_cores;
}

int detect_logical_cores() {
    int logical_cores;
    size_t len = sizeof(logical_cores);
    sysctlbyname("hw.logicalcpu", &logical_cores, &len, NULL, 0);
    return logical_cores;
}

#else 
#error "Unsupported platform for detecting cores and threads"
#endif

int detect_threads_per_core() {
    int physical_cores = detect_physical_cores();
    int logical_cores = detect_logical_cores();
    return (physical_cores > 0) ? (logical_cores / physical_cores) : 1;
}

struct sys_info {
    int cores;
    int threads;
    int threads_per_core;
};

struct sys_info print_sys_info() {
    struct sys_info info;
    info.cores = detect_physical_cores();
    info.threads = detect_logical_cores();
    info.threads_per_core = detect_threads_per_core();

    printf("System Information:\n");
    printf("  Physical Cores: %d\n", info.cores);
    printf("  Total Threads (Logical Cores): %d\n", info.threads);
    printf("  Threads per Core: %d\n", info.threads_per_core);

    return info;
}

#endif // TB_SYSINFO_H