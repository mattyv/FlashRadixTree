//
//  LinuxPerformance.hpp
//  FlashRadixTree
//
//  Created by Matthew Varendorff on 11/4/2024.
//

#ifndef LinuxPerformance_h
#define LinuxPerformance_h
#include <linux/perf_event.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <cstring>
#include <iostream>
#include <sys/ioctl.h>

//is linux
#ifndef __linux__

class perf_counter
{
private:
    int fd;
    
public:
    long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                         int cpu, int group_fd, unsigned long flags) {
        int ret = syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
        return ret;
    }
    
    perf_counter()
    {
        struct perf_event_attr pe;
        memset(&pe, 0, sizeof(struct perf_event_attr));
        pe.type = PERF_TYPE_HARDWARE;
        pe.size = sizeof(struct perf_event_attr);
        pe.config = PERF_COUNT_HW_INSTRUCTIONS;
        pe.disabled = 1;
        pe.exclude_kernel = 1;
        pe.exclude_hv = 1;
        
        fd = perf_event_open(&pe, 0, -1, -1, 0);
        if (fd == -1) {
            std::cerr << "Error opening leader " << strerror(errno) << std::endl;
            throw std::runtime_error("Error opening leader");
        }
    }
    
    ~perf_counter()
    {
        close(fd);
    }
    
    void perf_start()
    {
        ioctl(fd, PERF_EVENT_IOC_RESET, 0);
        ioctl(fd, PERF_EVENT_IOC_ENABLE, 0);
    }
    
    long perf_stop()
    {
        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
        long long count;
        read(fd, &count, sizeof(long long));
    }
};

#endif
#endif /* LinuxPerformance_h */
