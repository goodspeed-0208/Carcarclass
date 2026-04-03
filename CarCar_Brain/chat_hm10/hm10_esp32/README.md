# HM-10 ESP32 Bridge Interface

This Python module provides a high-level interface to interact with an ESP32 acting as a BLE-to-UART bridge. It allows a PC to communicate with an HM-10 Bluetooth module through an ESP32 by sending AT commands for configuration and raw data for GATT-based messaging.

---

## Features

* **AT Command Parsing**: Handles specific commands implemented in the ESP32 firmware, including `AT+NAME`, `AT+STATUS`, and `AT+RESET`.
* **Log Filtering**: Automatically strips ESP32 system logs and ANSI color escape codes to extract clean Bluetooth communication data.
* **Automatic Concatenation**: Merges fragmented data packets (caused by the ESP32's 10-byte UART buffer) into single, readable strings.
* **NVS Support**: Interfaces with the ESP32's Non-Volatile Storage to query or update the target device name.

---

## Hardware Requirements

* **ESP32**: Running the provided `main.c` firmware.
* **HM-10 Bluetooth Module**: Configured to be discoverable by the ESP32.
* **Connection**: ESP32 connected to PC via USB/UART (e.g., `/dev/ttyUSB2`).

---

## Installation

Ensure you have `pyserial` installed:

```bash
pip install pyserial

```

---

## API Reference

### `ESP32HM10Bridge(port, baudrate=115200)`

Initializes the serial connection. The default baud rate is **115200**.

### `get_status()`

Queries the ESP32 to check if it is currently connected to the HM-10.

* Returns: `"CONNECTED"`, `"DISCONNECTED"`, or `"TIMEOUT"`.
* Implementation: Sends `AT+STATUS?` and parses the `bt_com` log for `OK+CONN` or `OK+UNCONN`.

### `get_hm10_name()`

Queries the current target device name stored in the ESP32's NVS.

* Returns: The name string (e.g., `"HM10_Mega"`).

### `set_hm10_name(name)`

Updates the target HM-10 device name in the ESP32 NVS.

* Note: Requires an `AT+RESET` to take effect.

### `send(text)`

Sends raw text to the ESP32. If the ESP32 is connected to an HM-10, it forwards this data to the `0xFFE1` characteristic via GATT.

### `listen()`

Checks the UART buffer for incoming data from the HM-10. It filters out system logs and returns only the Bluetooth payload.

---

## Example Usage

### Simple Chat Script

This example demonstrates a threaded approach to handle real-time bidirectional communication.

```python
from hm10_esp32_bridge import ESP32HM10Bridge
import threading
import sys

def background_listener(bridge):
    while True:
        msg = bridge.listen()
        if msg:
            print(f"\r[HM10]: {msg}\nYou: ", end="")

bridge = ESP32HM10Bridge('/dev/ttyUSB2')

# Verify connection before starting
if bridge.get_status() == "CONNECTED":
    threading.Thread(target=background_listener, args=(bridge,), daemon=True).start()
    while True:
        val = input("You: ")
        bridge.send(val)
else:
    print("ESP32 is not connected to HM-10. Exiting.")
    sys.exit(0)

```

---

## Firmware Notes

The bridge relies on the following logic within the ESP32 `main.c`:

* **UART Buffer**: The firmware reads 10 bytes at a time.
* **Logging**: Communication data is logged with the tag `bt_com`.
* **BLE Service**: The bridge targets Service `0xFFE0` and Characteristic `0xFFE1`.
