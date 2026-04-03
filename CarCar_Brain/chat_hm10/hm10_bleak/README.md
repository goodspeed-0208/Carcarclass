## README: HM-10 Bleak Client

This module provides a robust, asynchronous GATT client designed to interface directly with **HM-10 (CC2541)** Bluetooth modules. It streamlines the connection process by abstracting the BLE scanning, MTU management, and data packetization into a simple `listen`/`send` interface.

---

### Key Features

* **16-bit UUID Support**: Uses simplified short-form UUIDs (`ffe0`/`ffe1`) with automatic 128-bit expansion.
* **Automatic Chunking**: Transparently splits messages longer than **20 bytes** to respect the BLE 4.0 MTU limit.
* **Serial-Style Buffering**: Includes an internal RX buffer so you can poll for data using a synchronous `listen()` call.
* **Smart Filtering**: Connects by matching device name, MAC address, or specifically searching for the HM-10 service UUID.

---

### Installation

#### 1. Miniconda 

[Install Miniconda](https://www.anaconda.com/docs/getting-started/miniconda/install) if you don't have it already.

Create a clean environment using **Python 3.12** (recommended version):

```bash
conda create -n car_env python=3.12 -y
conda activate car_env
pip install bleak

```

#### 2. venv (Standard Python)

```bash
python3.12 -m venv hm10_venv
source hm10_venv/bin/activate  # Windows: .\hm10_venv\Scripts\activate
pip install bleak

```

---

### API Reference

#### `HM10BleakClient(target_name=None, target_address=None)`

Initializes the client.

* **target_name**: String (e.g., `"HM10_Mega"`).
* **target_address**: MAC address (e.g., `"88:C2:55:03:BC:D7"`).

#### `await connect()`

Scans for the device using the provided name/address. Upon finding the device, it establishes a GATT connection and automatically starts listening to the `0xFFE1` characteristic.

* **Returns**: `True` if connected and notifications are enabled.

#### `listen()`

**Synchronous.** Returns all data accumulated in the internal buffer since the last call and clears the buffer.

* **Returns**: `String` (UTF-8 decoded).

#### `await send(message)`

Encodes the message and writes it to the HM-10.

* **MTU Management**: Automatically slices data into 20-byte chunks.
* **Flow Control**: Includes a 20ms delay between chunks to prevent UART buffer overflows on the receiving Arduino.

#### `await disconnect()`

Stops notifications and closes the BLE connection.

---

### Communication Logic

The module is designed for **Asynchronous Concurrency**. Because `Bleak` is push-based (notifications), the module populates an internal `_rx_buffer` in the background. The user script can then "pull" that data at its own pace.

---

### Example: Concurrent Chat Script

This pattern ensures the UI (typing) does not block the receipt of messages.

```python
import asyncio
import sys
from hm10_bleak_client import HM10BleakClient

async def main():
    hm10 = HM10BleakClient(target_name="HM10_Mega")
    if not await hm10.connect(): return

    # Define the Sender and Receiver tasks
    async def receiver():
        while True:
            msg = hm10.listen()
            if msg: print(f"\r[HM10]: {msg}\nYou: ", end="", flush=True)
            await asyncio.sleep(0.1)

    async def sender():
        loop = asyncio.get_running_loop()
        while True:
            print("You: ", end="", flush=True)
            msg = await loop.run_in_executor(None, sys.stdin.readline)
            if msg.strip().lower() in ['exit', 'quit']: return
            await hm10.send(msg)

    # Run tasks concurrently; stop when sender returns 'exit'
    st = asyncio.create_task(sender())
    rt = asyncio.create_task(receiver())
    
    await st
    rt.cancel()
    await hm10.disconnect()

if __name__ == "__main__":
    asyncio.run(main())

```
