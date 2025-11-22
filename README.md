# CharPulse

**CharPulse** is a **Linux character device driver** that supports **read, write, append, and clear** operations on a kernel buffer. It is safe for multi-threaded access and includes logging for all operations via `dmesg`. This driver is suitable for learning and experimenting with kernel modules.

---

## Features

- Read data from the device.
- Write data to the device.
- Append data to the existing buffer.
- Clear the buffer using `echo "clear" > /dev/charpulse`.
- Thread-safe with mutex protection.
- Production-ready with logging via `dmesg`.
- Auto-loads at boot using `/etc/modules-load.d/charpulse.conf`.
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
4. **Build and install**
```bash
sudo ./install.sh
```
5. **Set device permissions (optional):**
```bash
sudo chmod 666 /dev/charpulse
```
4. **Verify the module is loaded:**
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
echo "Hello" > /dev/charpulse
```
**Append data:**
```bash
echo " World" >> /dev/charpulse
```
**Read data:**
```bash
cat /dev/charpulse
```
**Clear buffer:**
```bash
echo "clear" > /dev/charpulse
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
sudo depmod -a
```

---

## License
This project is licensed under the GNU General Public License v3.0
