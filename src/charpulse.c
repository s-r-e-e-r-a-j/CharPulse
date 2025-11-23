// Developer: Sreeraj
// GitHub: https://github.com/s-r-e-e-r-a-j

#include "../include/charpulse.h"

static dev_t cp_dev;
static struct cdev cp_cdev;
static struct class *cp_class;
static struct device *cp_device;
static char *cp_buf;
static size_t cp_len = 0;
static size_t buffer_size = CHARPULSE_BUF;
static DEFINE_MUTEX(cp_lock);

static struct file_operations cp_ops = {
    .owner   = THIS_MODULE,
    .open    = cp_open,
    .release = cp_close,
    .read    = cp_read,
    .write   = cp_write,
    .llseek  = cp_llseek,
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
                memset(cp_buf, 0, buffer_size);
                cp_len = 0;
                *off = 0;
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
        case SEEK_SET:
            newpos = off;
            break;
        case SEEK_CUR:
            newpos = file->f_pos + off;
            break;
        case SEEK_END:
            newpos = cp_len + off;
            break;
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

    printk(KERN_INFO "charpulse: module loaded successfully\n");
    return 0;
}

void cp_exit(void) {
    printk(KERN_INFO "charpulse: unloading module...\n");
    kfree(cp_buf);
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
MODULE_DESCRIPTION("CharPulse character device driver with read, write, append, clear, and dynamic buffer resizing support");

