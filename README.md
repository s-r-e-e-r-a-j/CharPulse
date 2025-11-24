# CharPulse

**CharPulse** is a Linux character device driver that supports **read**, **write**, **append**, and **clear** operations on a **dynamically resizing kernel buffer**. It is safe for **multi-threaded access** and includes **logging for all operations via `dmesg`**.  

This driver also provides **sysfs support** to monitor and manage buffer operations, including **read_count**, **write_count**, **clear_count**, **current_data_size**, and **reset_counts** to reset all counters.  

This driver is **production-ready** and can handle **large amounts of data**, making it suitable for **learning** as well as **realistic kernel module experiments**.

---

## Features

- Read data from the device.
- Write data to the device.
- Append data to the existing buffer.
- Clear the buffer using `echo "clear" > /dev/charpulse`.
- Thread-safe with mutex protection.
- Dynamic, auto-resizing kernel buffer.
- Production-ready with logging via `dmesg`.
- Auto-loads at boot using `/etc/modules-load.d/charpulse.conf`.
- Sysfs attributes for monitoring and control:
  - `read_count` – number of times data has been read
  - `write_count` – number of times data has been written
  - `clear_count` – number of times the buffer has been cleared
  - `current_data_size` – current size of the data in the buffer
  - `reset_counts` – write `1` to reset read, write, and clear counters
- Works on most Linux distributions with proper kernel headers.

---

## Requirements

- Linux kernel 6.14 or newer  
- GCC 13 or newer, or the GCC version used to build your running kernel  
- Make  
- Linux kernel headers for the running kernel (usually preinstalled on Ubuntu)
> **Note:** Tested on Linux kernel 6.14.0-35-generic (Linux kernel 6.14) on Ubuntu 24.04.3 LTS

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
