// charpulse_user.h - Userspace IOCTL interface for CharPulse 

#ifndef CHARPULSE_USER_H
#define CHARPULSE_USER_H

#include <stdint.h>
#include <stddef.h>
#include <sys/ioctl.h>

#define CHARPULSE_IOC_MAGIC 'c'

#define CP_CLEAR_BUFFER        _IO(CHARPULSE_IOC_MAGIC, 1)
#define CP_GET_STATS           _IOR(CHARPULSE_IOC_MAGIC, 2, struct cp_stats)
#define CP_SET_MAX_SIZE        _IOW(CHARPULSE_IOC_MAGIC, 3, size_t)
#define CP_GET_BUFFER_USAGE    _IOR(CHARPULSE_IOC_MAGIC, 4, char *)

struct cp_stats {
    uint64_t read_count;
    uint64_t write_count;
    uint64_t clear_count;
    size_t last_read_size;
    size_t last_write_size;
    size_t current_data_size;
};

#endif
