#!/usr/bin/env bash
set -e

MODULE_BASENAME="charpulse"
MODULE_DISPLAY="CharPulse"
MODULE_FILE="${MODULE_BASENAME}.ko"
DEV_NODE="/dev/${MODULE_BASENAME}"

echo "[*] Building ${MODULE_DISPLAY} module..."
make

echo "[*] Creating module directory..."
sudo mkdir -p /lib/modules/$(uname -r)/extra

echo "[*] Installing ${MODULE_FILE}..."
sudo cp "${MODULE_FILE}" /lib/modules/$(uname -r)/extra/

echo "[*] Running depmod..."
sudo depmod -a

echo "[*] Enabling auto-load at boot..."
echo "${MODULE_BASENAME}" | sudo tee /etc/modules-load.d/${MODULE_BASENAME}.conf > /dev/null

echo "[*] Loading module now..."
if sudo modprobe "${MODULE_BASENAME}"; then
    echo "[+] modprobe succeeded"
else
    echo "[*] modprobe failed, trying insmod..."
    sudo insmod "/lib/modules/$(uname -r)/extra/${MODULE_FILE}"
fi

echo "[*] Waiting for udev to create device node..."
sudo udevadm settle --exit-if-exists="${DEV_NODE}" || true

if [ -e "${DEV_NODE}" ]; then
    echo "[*] Setting permissions 0666 on ${DEV_NODE}..."
    sudo chmod 0666 "${DEV_NODE}"
    echo "[*] Creating udev rule for permanent permissions..."
    sudo tee /etc/udev/rules.d/99-${MODULE_BASENAME}.rules > /dev/null <<EOF
    KERNEL=="${MODULE_BASENAME}", MODE="0666"
    EOF

    echo "[*] Reloading udev rules..."
    sudo udevadm control --reload
    sudo udevadm trigger

    echo "[+] ${MODULE_DISPLAY} installed and ${DEV_NODE} is ready (permissions 0666)"
else
    echo "[!] Warning: ${DEV_NODE} not found. You may need to check dmesg or udev rules."
    echo "         Check with: dmesg | tail"
fi

echo "[*] Verifying module is loaded..."
if lsmod | grep -q "^${MODULE_BASENAME}"; then
    echo "[+] ${MODULE_DISPLAY} is loaded"
else
    echo "[!] ${MODULE_DISPLAY} not listed in lsmod; check dmesg"
fi

echo "[*] Installation complete!"
