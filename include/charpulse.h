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

#define CHARPULSE_NAME "charpulse"
#define CHARPULSE_BUF 1024

int cp_open(struct inode *inode, struct file *file);
int cp_close(struct inode *inode, struct file *file);
ssize_t cp_read(struct file *file, char __user *out, size_t len, loff_t *off);
ssize_t cp_write(struct file *file, const char __user *in, size_t len, loff_t *off);
loff_t cp_llseek(struct file *file, loff_t off, int whence);

int cp_init(void);
void cp_exit(void);

#endif

