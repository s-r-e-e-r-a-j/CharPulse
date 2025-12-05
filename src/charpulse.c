// Developer: Sreeraj
// GitHub: https://github.com/s-r-e-e-r-a-j

#include "../include/charpulse.h"
#include <linux/kobject.h>
#include <linux/sysfs.h>

static dev_t cp_dev;
static struct cdev cp_cdev;
static struct class *cp_class;
static struct device *cp_device;
static char *cp_buf;
static size_t cp_len = 0;
static size_t buffer_size = CHARPULSE_BUF;
static size_t buffer_capacity;
static DEFINE_MUTEX(cp_lock);
static DECLARE_WAIT_QUEUE_HEAD(cp_wait);

static u64 read_count = 0;
static u64 write_count = 0;
static u64 clear_count = 0;
static size_t last_write_size = 0;
static size_t last_read_size = 0;

static struct kobject *cp_kobj;

static struct file_operations cp_ops = {
    .owner   = THIS_MODULE,
    .open    = cp_open,
    .release = cp_close,
    .read    = cp_read,
    .write   = cp_write,
    .llseek  = cp_llseek,
    .unlocked_ioctl = cp_ioctl,
    .poll = cp_poll,
};

int cp_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "charpulse: device opened\n");
    return 0;
}

int cp_close(struct inode *inode, struct file *file) {
    printk(KERN_INFO "charpulse: device closed\n");
    return 0;
}

ssize_t cp_read(struct file *file, char __user *out, size_t len, loff_t *off) {
    ssize_t ret = 0;
    if (mutex_lock_interruptible(&cp_lock))
        return -EINTR;

    if (*off >= cp_len) {
        ret = 0;
        goto out;
    }
    if (len > cp_len - *off)
        len = cp_len - *off;

    if (copy_to_user(out, cp_buf + *off, len)) {
        ret = -EFAULT;
        goto out;
    }

    *off += len;
    ret = len;
    last_read_size = len;
    read_count++;

out:
    mutex_unlock(&cp_lock);
    printk(KERN_INFO "charpulse: read %zd bytes\n", ret);
    return ret;
}

ssize_t cp_write(struct file *file, const char __user *in, size_t len, loff_t *off) {
    ssize_t ret = 0;
    loff_t pos;
    char kbuf[16] = {0};

    if (mutex_lock_interruptible(&cp_lock))
        return -EINTR;

    if (len <= 7) {
        if (copy_from_user(kbuf, in, len) == 0) {
            kbuf[len] = '\0';
            if (kbuf[len - 1] == '\n') kbuf[len - 1] = '\0';
            if (kbuf[len - 1] == '\r') kbuf[len - 1] = '\0';

            if (strcmp(kbuf, "clear") == 0) {
                kfree(cp_buf);
                buffer_size = CHARPULSE_BUF;
                cp_buf = kmalloc(buffer_size, GFP_KERNEL);
                if (!cp_buf) {
                   mutex_unlock(&cp_lock);
                   return -ENOMEM;
                }
                memset(cp_buf, 0, buffer_size);
                cp_len = 0;
                *off = 0;
                buffer_capacity = buffer_size;
                clear_count++;
                wake_up_interruptible(&cp_wait);
                mutex_unlock(&cp_lock);
                printk(KERN_INFO "charpulse: buffer cleared\n");
                return len;
            }
        }
    }

    if (file->f_flags & O_APPEND)
        pos = cp_len;
    else
        pos = *off;

    if (pos < 0) {
        ret = -EINVAL;
        goto out;
    }

    if (pos + len > buffer_size) {
        size_t new_size = max(buffer_size * 2, pos + len);
        char *new_buf = krealloc(cp_buf, new_size, GFP_KERNEL);
        if (!new_buf) {
           mutex_unlock(&cp_lock);
           return -ENOMEM;
        }
        cp_buf = new_buf;
        buffer_size = new_size;
        buffer_capacity = new_size;
    }

    if (copy_from_user(cp_buf + pos, in, len)) {
        ret = -EFAULT;
        goto out;
    }

    pos += len;
    if (pos > cp_len)
        cp_len = pos;

    *off = pos;
    ret = len;
    last_write_size = len;
    write_count++;
    wake_up_interruptible(&cp_wait);
out:
    mutex_unlock(&cp_lock);
    printk(KERN_INFO "charpulse: wrote %zd bytes (append=%d)\n",
           ret, !!(file->f_flags & O_APPEND));
    return ret;
}

loff_t cp_llseek(struct file *file, loff_t off, int whence) {
    loff_t newpos;
    mutex_lock(&cp_lock);

    switch (whence) {
        case SEEK_SET: newpos = off; break;
        case SEEK_CUR: newpos = file->f_pos + off; break;
        case SEEK_END: newpos = cp_len + off; break;
        default:
            mutex_unlock(&cp_lock);
            return -EINVAL;
    }

    if (newpos < 0 || newpos > cp_len) {
        mutex_unlock(&cp_lock);
        return -EINVAL;
    }

    file->f_pos = newpos;
    mutex_unlock(&cp_lock);
    return newpos;
}

unsigned int cp_poll(struct file *file, poll_table *wait) {
    unsigned int mask = 0;
    poll_wait(file, &cp_wait, wait);

    if (cp_len > 0)
        mask |= POLLIN | POLLRDNORM;

    return mask;
}

long cp_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    long ret = 0;

    if (mutex_lock_interruptible(&cp_lock))
        return -EINTR;

    switch (cmd) {
        case CP_CLEAR_BUFFER:
            kfree(cp_buf);
            buffer_size = CHARPULSE_BUF;
            cp_buf = kmalloc(buffer_size, GFP_KERNEL);
            if (!cp_buf) { ret = -ENOMEM; break; }
            memset(cp_buf, 0, buffer_size);
            cp_len = 0;
            buffer_capacity = buffer_size;
            clear_count++;
            wake_up_interruptible(&cp_wait);
            break;

        case CP_GET_STATS: {
            struct cp_stats stats = {
                .read_count = read_count,
                .write_count = write_count,
                .clear_count = clear_count,
                .last_read_size = last_read_size,
                .last_write_size = last_write_size,
                .current_data_size = cp_len
            };
            if (copy_to_user((void __user *)arg, &stats, sizeof(stats)))
                ret = -EFAULT;
            break;
        }

        case CP_SET_MAX_SIZE: {
            size_t new_size;
            if (copy_from_user(&new_size, (size_t __user *)arg, sizeof(new_size))) {
                ret = -EFAULT;
                break;
            }
            if (new_size >= cp_len)
                buffer_capacity = new_size;
            else
                ret = -EINVAL;
            break;
        }

        case CP_GET_BUFFER_USAGE: {
             char buf[16];
             size_t len;
             u64 usage_scaled;
             u64 percent_int, percent_dec;

             if (buffer_capacity == 0) {
                percent_int = 0;
                percent_dec = 0;
             } else {
                  usage_scaled = (u64)cp_len * 10000 / buffer_capacity;
                  percent_int = usage_scaled / 100;
                  percent_dec = usage_scaled % 100;
             }


             len = snprintf(buf, sizeof(buf), "%llu.%02llu", percent_int, percent_dec);

             if (copy_to_user((char __user *)arg, buf, len + 1))
                ret = -EFAULT;

             break;
        }


        default:
            ret = -ENOTTY;
    }

    mutex_unlock(&cp_lock);
    return ret;
}

static ssize_t read_count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%llu\n", read_count);
}

static ssize_t write_count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%llu\n", write_count);
}

static ssize_t clear_count_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%llu\n", clear_count);
}

static ssize_t current_data_size_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%zu\n", cp_len);
}

static ssize_t last_write_size_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%zu\n", last_write_size);
}

static ssize_t last_read_size_show(struct kobject *kobject, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%zu\n", last_read_size);
}

static ssize_t buffer_usage_percentage_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    size_t base_capacity;
    u64 usage_scaled;
    u64 percent_int;
    u64 percent_dec;

    if (mutex_lock_interruptible(&cp_lock))
        return -EINTR;

    base_capacity = buffer_capacity;

    if (base_capacity == 0) {
        mutex_unlock(&cp_lock);
        return sprintf(buf, "0.00\n");
    }

    usage_scaled = (u64)cp_len * 10000ULL / base_capacity;

    percent_int = usage_scaled / 100;
    percent_dec = usage_scaled % 100;

    mutex_unlock(&cp_lock);

    return sprintf(buf, "%llu.%02llu\n", percent_int, percent_dec);
}

static ssize_t reset_counts_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    read_count = 0;
    write_count = 0;
    clear_count = 0;
    return count;
}

static struct kobj_attribute read_count_attr = __ATTR_RO(read_count);
static struct kobj_attribute write_count_attr = __ATTR_RO(write_count);
static struct kobj_attribute clear_count_attr = __ATTR_RO(clear_count);
static struct kobj_attribute current_data_size_attr = __ATTR_RO(current_data_size);
static struct kobj_attribute last_write_size_attr = __ATTR_RO(last_write_size);
static struct kobj_attribute last_read_size_attr  = __ATTR_RO(last_read_size);
static struct kobj_attribute buffer_usage_percentage_attr = __ATTR_RO(buffer_usage_percentage);
static struct kobj_attribute reset_counts_attr = __ATTR_WO(reset_counts);

static struct attribute *cp_attrs[] = {
    &read_count_attr.attr,
    &write_count_attr.attr,
    &clear_count_attr.attr,
    &current_data_size_attr.attr,
    &last_write_size_attr.attr,
    &last_read_size_attr.attr,
    &buffer_usage_percentage_attr.attr,
    &reset_counts_attr.attr,
    NULL,
};

static struct attribute_group cp_attr_group = {
    .attrs = cp_attrs,
};

int cp_init(void) {
    int ret;

    printk(KERN_INFO "charpulse: loading module...\n");

    ret = alloc_chrdev_region(&cp_dev, 0, 1, CHARPULSE_NAME);
    if (ret)
        return ret;

    cdev_init(&cp_cdev, &cp_ops);
    cp_cdev.owner = THIS_MODULE;

    ret = cdev_add(&cp_cdev, cp_dev, 1);
    if (ret) {
        unregister_chrdev_region(cp_dev, 1);
        return ret;
    }

    cp_class = class_create(CHARPULSE_NAME);
    if (IS_ERR(cp_class)) {
        ret = PTR_ERR(cp_class);
        cdev_del(&cp_cdev);
        unregister_chrdev_region(cp_dev, 1);
        return ret;
    }

    cp_device = device_create(cp_class, NULL, cp_dev, NULL, "%s", CHARPULSE_NAME);
    if (IS_ERR(cp_device)) {
        ret = PTR_ERR(cp_device);
        class_destroy(cp_class);
        cdev_del(&cp_cdev);
        unregister_chrdev_region(cp_dev, 1);
        return ret;
    }

    mutex_init(&cp_lock);
    cp_buf = kmalloc(buffer_size, GFP_KERNEL);
    if (!cp_buf) {
        device_destroy(cp_class, cp_dev);
        class_destroy(cp_class);
        cdev_del(&cp_cdev);
        unregister_chrdev_region(cp_dev, 1);
        return -ENOMEM;
    }
    memset(cp_buf, 0, buffer_size);
    cp_len = 0;
    buffer_capacity = buffer_size;

    cp_kobj = kobject_create_and_add("charpulse_stats", kernel_kobj);
    if (!cp_kobj)
        return -ENOMEM;

    ret = sysfs_create_group(cp_kobj, &cp_attr_group);
    if (ret)
        kobject_put(cp_kobj);

    printk(KERN_INFO "charpulse: module loaded successfully\n");
    return 0;
}

void cp_exit(void) {
    printk(KERN_INFO "charpulse: unloading module...\n");
    kfree(cp_buf);
    if (cp_kobj)
        kobject_put(cp_kobj);
    device_destroy(cp_class, cp_dev);
    class_destroy(cp_class);
    cdev_del(&cp_cdev);
    unregister_chrdev_region(cp_dev, 1);
    printk(KERN_INFO "charpulse: module unloaded\n");
}

module_init(cp_init);
module_exit(cp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sreeraj S Kurup");
MODULE_DESCRIPTION("CharPulse character device driver with read, write, append, clear, dynamic buffer resizing, and sysfs stats, poll support, and IOCTL interface for stats and buffer management");
