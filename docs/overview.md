# LL-Connect (Linux port of L-Connect 3)

This project ports L-Connect 3 functionality to Linux for Lian Li devices. The goal is to drive lighting, fan control, and other features through a native Linux userspace application and optional kernel module(s), based on reverse-engineered USB protocols.

## Repo layout (high level)
- Root C sources: userspace controller and UI
- `kernel/`: out-of-tree kernel module(s) and build scripts
- `docs/`: documentation and capture notes

## Reverse engineering approach
1. Record device traffic on Windows using official L-Connect 3 and Wireshark/USBPcap.
2. Identify endpoints, setup requests, and vendor-specific commands.
3. Map commands to observed behavior (e.g., color, brightness, speed).
4. Implement equivalent commands on Linux (hidraw/libusb or kernel driver).
5. Validate against multiple devices/firmwares.

## Related docs
- [Wireshark Capture Guide (Windows)](wireshark-windows.md)
- [Capture Log Template](templates/capture-log-template.md)
- [UNI HUB AL v2 Protocol Reference](protocols/uni-hub-alv2-protocol.md)
- (Planned) Linux Porting Workflow
