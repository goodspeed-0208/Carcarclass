import serial
import time
import re

class HM10ESP32Bridge:
    def __init__(self, port, rx_timeout=0.1):
        self.ser = serial.Serial(port=port, baudrate=115200, timeout=rx_timeout)
        self.log_regex = re.compile(r'bt_com:\s*(.*)')
        self.ansi_regex = re.compile(r'\x1b\[[0-9;]*m')
        
        self._rx_buffer = "" # 【新增】持續性緩衝區
        time.sleep(1)

    def _read_bt_com_payloads(self):
        if self.ser.in_waiting == 0:
            return []
        raw_data = self.ser.read_all().decode('utf-8', errors='ignore')
        # 【修改】保留換行符號 (keepends=True)
        lines = raw_data.splitlines(keepends=True) 
        payloads = []
        for line in lines:
            match = self.log_regex.search(line)
            if match:
                # 【修改】移除 .strip()，保留原始的 \r 或 \n
                clean_payload = self.ansi_regex.sub('', match.group(1))
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
        """Returns completely assembled lines separated by \\n."""
        logs = self._read_bt_com_payloads()
        data_parts = [l for l in logs if not l.startswith("OK+")]
        
        # 把新收到的碎片全部黏到緩衝區尾巴
        self._rx_buffer += "".join(data_parts)
        
        # 只要緩衝區裡有換行符號，就切出一行完整的回傳
        if '\n' in self._rx_buffer:
            line, self._rx_buffer = self._rx_buffer.split('\n', 1)
            return line.strip()
            
        return None

    def send(self, text):
        """Sends data to be forwarded to HM-10 via GATT."""
        self.ser.write(text.encode('utf-8'))