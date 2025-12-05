// Developer: Sreeraj
// GitHub: https://github.com/s-r-e-e-r-a-j

#ifndef CHARPULSE_H
#define CHARPULSE_H

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/poll.h>

#define CHARPULSE_NAME "charpulse"
#define CHARPULSE_BUF 1024

#define CHARPULSE_IOC_MAGIC 'c'

#define CP_CLEAR_BUFFER    _IO(CHARPULSE_IOC_MAGIC, 1)
#define CP_GET_STATS       _IOR(CHARPULSE_IOC_MAGIC, 2, struct cp_stats)
#define CP_GET_BUFFER_USAGE  _IOR(CHARPULSE_IOC_MAGIC, 4, char*)

struct cp_stats {
    u64 read_count;
    u64 write_count;
    u64 clear_count;
    size_t last_read_size;
    size_t last_write_size;
    size_t current_data_size;
};

int cp_open(struct inode *inode, struct file *file);
int cp_close(struct inode *inode, struct file *file);
ssize_t cp_read(struct file *file, char __user *out, size_t len, loff_t *off);
ssize_t cp_write(struct file *file, const char __user *in, size_t len, loff_t *off);
loff_t cp_llseek(struct file *file, loff_t off, int whence);
long cp_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
unsigned int cp_poll(struct file *file, poll_table *wait);

int cp_init(void);
void cp_exit(void);

#endif
