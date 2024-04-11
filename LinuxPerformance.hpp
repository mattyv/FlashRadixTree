//
//  LinuxPerformance.hpp
//  FlashRadixTree
//
//  Created by Matthew Varendorff on 11/4/2024.
//

#ifndef LinuxPerformance_h
#define LinuxPerformance_h
#include <vector>
//is linux
#ifdef __linux__
#include <linux/perf_event.h>
#include <unistd.h>
#include <asm/unistd.h>
#include <cstring>
#include <iostream>
#include <sys/ioctl.h>
#include <limits>


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
    
    long long perf_stop()
    {
        ioctl(fd, PERF_EVENT_IOC_DISABLE, 0);
        long long count;
        read(fd, &count, sizeof(long long));
	return count;
    }
};

#endif

class perf_counter_holder
{
private:
    std::vector<long long> counters;
    bool sorted = false;
    
    void _sort()
    {
        std::sort(counters.begin(), counters.end());
        sorted = true;
    }
public:
    perf_counter_holder() = default;
    
    perf_counter_holder operator +=(long long counter)
    {
        counters.push_back(counter);
        sorted = false;
        return *this;
    }
    
    long long average() const
    {
        long sum = 0;
        for (auto counter : counters)
        {
            sum += counter;
        }
        
        return sum / counters.size();
    }
    
    long long min() const
    {
        long long min = std::numeric_limits<long long>::max();
        for (auto counter : counters)
        {
            if (counter < min)
            {
                min = counter;
            }
        }
        
        return min;
    }
    
    long long max()
    {
        long long max = std::numeric_limits<long long>::min();
        for (auto counter : counters)
        {
            if (counter > max)
            {
                max = counter;
            }
        }
        
        return max;
    }
    
    long long median()
    {
        if (!sorted)
        {
            _sort();
        }
        
        return counters[counters.size() / 2];
    }
    
    long long percentile(double p)
    {
        if (!sorted)
        {
            _sort();
        }
        
        return counters[counters.size() * p];
    }
    
    void clear()
    {
        counters.clear();
        sorted = false;
    }
    
    
};

#endif /* LinuxPerformance_h */
