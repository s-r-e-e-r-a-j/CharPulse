# CharPulse
**CharPulse** is a Linux character device driver that supports **read**, **write**, **append**, and **clear** operations on a **dynamically resizing kernel buffer**. It is safe for **multi-threaded access** and includes **logging for all operations via `dmesg`**.  
This driver also provides **sysfs support** to monitor and manage buffer operations, including **read_count**, **write_count**, **clear_count**, **current_data_size**, **last_write_size**, **last_read_size**, **buffer_usage_percentage** and **reset_counts** to reset all counters.  
It also supports **poll** to check if data is available and an **IOCTL interface** for getting stats, clearing the buffer, setting maximum buffer size, and checking buffer usage.  
This driver is **production-ready** and can handle **large amounts of data**, making it suitable for **learning** as well as **realistic kernel module experiments**.

---

## Features

- Read data from the device.
- Write data to the device.
- Append data to the existing buffer.
- Clear all data using `echo "clear" > /dev/charpulse` (buffer is reallocated to initial size)
- Thread-safe with mutex protection.
- Dynamic, auto-resizing kernel buffer.
- Production-ready with logging via `dmesg`.
- Auto-loads at boot using `/etc/modules-load.d/charpulse.conf`.
- Sysfs attributes for monitoring and control:
  - `read_count` – number of times data has been read
  - `write_count` – number of times data has been written
  - `clear_count` – number of times the buffer has been cleared
  - `current_data_size` – current size of the data in the buffer
  - `last_write_size` – size of the last write operation
  - `last_read_size` – size of the last read operation
  - `buffer_usage_percentage` – percentage of the buffer currently in use
  - `reset_counts` – write `1` to reset read, write, and clear counters
 
- Supports **poll()** to check if data is available for reading.
- Supports **IOCTL interface** to get stats, clear the buffer, set maximum buffer size, and check buffer usage.
- Works on most Linux distributions with proper kernel headers.

---

## Requirements

- **Linux kernel 6.14 or newer**
  - Tested on **6.14, 6.15, 6.16, 6.17, and 6.18** on Ubuntu 24.04.3 LTS

- **GCC 13 or newer**, or the GCC version used to build your running kernel
  - **For mainline kernels 6.15 and 6.16, GCC 14 is required**
  - **GCC 14 must be accessible as `/usr/bin/gcc-14`** (symlinked if installed elsewhere)  
    - Simply naming it `gcc` is not enough
  - **For mainline kernels 6.17 and 6.18, GCC 15 is required**
  - **GCC 15 must be accessible as `/usr/bin/gcc-15`** (symlinked if installed elsewhere)  
    - Simply naming it `gcc` is not enough

- **Make**
- **Linux kernel headers** for the running kernel (usually preinstalled on Ubuntu)

**Note for mainline kernels 6.15 and 6.16:**  
`/usr/bin/gcc-14` symlink is mandatory. Without it, you may get compilation errors.

**Note for mainline kernels 6.17 and 6.18:**  
`/usr/bin/gcc-15` symlink is mandatory. Without it, you may get compilation errors.

---
  
## Installation

1. **Clone the repository:**
```bash
git clone https://github.com/s-r-e-e-r-a-j/CharPulse.git
 ```
2. **Go to the CharPulse directory:**
```bash
cd CharPulse
```
3. **Give execute permission to the `install.sh` script:**
```bash
chmod +x install.sh
```
4. **Build and install:**
```bash
sudo ./install.sh
```
5. **Verify the module is loaded:**
```bash
lsmod | grep charpulse
```
```bash
dmesg | tail
```

---

## Usage

**Write data:**
```bash
echo "Hello1" > /dev/charpulse
```
**Append data:**
```bash
echo "Hello2" >> /dev/charpulse
```
**Read data:**
```bash
cat /dev/charpulse
```
**Clear buffer:**  
Clears all data from the buffer and reallocates it to the initial size. 
```bash
echo "clear" > /dev/charpulse
```
### Sysfs Usage

**Read Count**

Check how many times the device has been read:
```bash
cat /sys/kernel/charpulse_stats/read_count
```
**Write Count**

Check how many times the device has been written to:
```bash
cat /sys/kernel/charpulse_stats/write_count
```
**Clear Count**

Check how many times the buffer has been cleared:
```bash
cat /sys/kernel/charpulse_stats/clear_count
```
**Current Data Size**

Check the current size of data stored in the buffer (in bytes):
```bash
cat /sys/kernel/charpulse_stats/current_data_size
```
**Last Write Size**

Check the size of the last write operation (in bytes):
```bash
cat /sys/kernel/charpulse_stats/last_write_size
```
**Last Read Size**

Check the size of the last read operation (in bytes):
```bash
cat /sys/kernel/charpulse_stats/last_read_size
```
**Buffer Usage Percentage**

Check the current buffer usage as a percentage:
```bash
cat /sys/kernel/charpulse_stats/buffer_usage_percentage
```
**Reset Counts**

Reset read, write, and clear counts to zero (requires sudo/root):
```bash
sudo sh -c 'echo 1 > /sys/kernel/charpulse_stats/reset_counts'
```

---

## Uninstallation
**Run each command separately to avoid errors:**
```bash
sudo rmmod charpulse
```
```bash
sudo rm /etc/modules-load.d/charpulse.conf
```
```bash
sudo rm /lib/modules/$(uname -r)/extra/charpulse.ko
```
```bash
sudo rm /etc/udev/rules.d/99-charpulse.rules
```
```bash
sudo depmod -a
```

---

## License
This project is licensed under the GNU General Public License v3.0
