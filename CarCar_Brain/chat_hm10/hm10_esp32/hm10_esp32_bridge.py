import serial
import time
import re

class HM10ESP32Bridge:
    def __init__(self, port, rx_timeout=0.1):
        self.ser = serial.Serial(port=port, baudrate=115200, timeout=rx_timeout)
        # Matches 'bt_com' tag logs from ESP32
        self.log_regex = re.compile(r'bt_com:\s*(.*)')
        # Strips ANSI color codes often sent by ESP-IDF
        self.ansi_regex = re.compile(r'\x1b\[[0-9;]*m')
        time.sleep(1) 

    def _read_bt_com_payloads(self):
        """Reads and cleans all 'bt_com' tagged logs currently in buffer."""
        if self.ser.in_waiting == 0:
            return []
        raw_data = self.ser.read_all().decode('utf-8', errors='ignore')
        lines = raw_data.splitlines()
        payloads = []
        for line in lines:
            match = self.log_regex.search(line)
            if match:
                # Clean ANSI colors and whitespace
                clean_payload = self.ansi_regex.sub('', match.group(1)).strip()
                payloads.append(clean_payload)
        return payloads

    def set_hm10_name(self, name, timeout=2.0):
        """
        Sends AT+NAME<name> and verifies OK+SET<name> reply.
        Returns True on success, False on timeout/failure.
        """
        command = f"AT+NAME{name}"
        self.ser.write(command.encode('utf-8'))
        
        # Poll for the specific OK+SET response
        start_time = time.time()
        while (time.time() - start_time) < timeout:
            for entry in self._read_bt_com_payloads():
                if f"OK+SET{name}" in entry:
                    return True
            time.sleep(0.01)
        return False

    def get_hm10_name(self, timeout=2.0):
        """Queries the device name currently in NVS."""
        self.ser.write(b"AT+NAME?")
        start_time = time.time()
        while (time.time() - start_time) < timeout:
            for entry in self._read_bt_com_payloads():
                if "OK+NAME" in entry:
                    return entry.replace("OK+NAME", "").strip()
            time.sleep(0.01)
        return None

    def get_status(self, timeout=2.0):
        """Checks connection status via AT+STATUS?."""
        self.ser.write(b"AT+STATUS?")
        start_time = time.time()
        while (time.time() - start_time) < timeout:
            for entry in self._read_bt_com_payloads():
                if "OK+CONN" in entry: return "CONNECTED"
                if "OK+UNCONN" in entry: return "DISCONNECTED"
            time.sleep(0.01)
        return "TIMEOUT"

    def reset(self):
        """Triggers AT+RESET and returns True if OK+RESET is received."""
        self.ser.write(b"AT+RESET")
        start_time = time.time()
        while (time.time() - start_time) < 10.0:
            for entry in self._read_bt_com_payloads():
                if "OK+RESET" in entry:
                    time.sleep(6) # Wait for ESP32 to reboot and connect to HM-10
                    return True
            time.sleep(0.01)
        return False

    def listen(self):
        """Returns concatenated data from BLE (ignores AT replies)."""
        logs = self._read_bt_com_payloads()
        data_parts = [l for l in logs if not l.startswith("OK+")]
        return "".join(data_parts)

    def send(self, text):
        """Sends data to be forwarded to HM-10 via GATT."""
        self.ser.write(text.encode('utf-8'))