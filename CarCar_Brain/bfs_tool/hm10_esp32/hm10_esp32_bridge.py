import serial
import time
import re

class HM10ESP32Bridge:
    def __init__(self, port, rx_timeout=0.1):
        self.ser = serial.Serial(port=port, baudrate=115200, timeout=rx_timeout)
        self.log_regex = re.compile(r'bt_com:\s*(.*)')
        self.ansi_regex = re.compile(r'\x1b\[[0-9;]*m')
        
        self._rx_buffer = ""
        self._raw_serial_buffer = ""  # 【新增】用來接合被切斷的 UART 封包
        time.sleep(1)

    def _read_bt_com_payloads(self):
        if self.ser.in_waiting == 0:
            return []
        raw_data = self.ser.read_all().decode('utf-8', errors='ignore')
        self._raw_serial_buffer += raw_data
        
        # 切割 ESP32 的 Log
        lines = self._raw_serial_buffer.split('\n')
        self._raw_serial_buffer = lines.pop() 

        payloads = []
        current_payload = None # 用來暫存並接合跨行的藍牙封包
        
        for line in lines:
            if "bt_com:" in line:
                # 遇到新的 bt_com:，先把上一個已經組合好的 payload 存進陣列
                if current_payload is not None:
                    clean_payload = self.ansi_regex.sub('', current_payload)
                    payloads.append(clean_payload)
                
                # 建立新的 payload
                current_payload = line.split("bt_com:", 1)[1]
                if current_payload.startswith(" "):
                    current_payload = current_payload[1:]
            else:
                # 關鍵修復點！
                # 如果這行沒有 bt_com:，代表它是被 Arduino 的 \n 切斷的封包下半部！
                # 我們把它跟前面的 payload 黏回去，並補回被 split('\n') 吃掉的 \n
                if current_payload is not None:
                    current_payload += '\n' + line

        # 處理迴圈最後一個尚未存入的 payload
        if current_payload is not None:
            clean_payload = self.ansi_regex.sub('', current_payload)
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
        """Returns completely assembled lines separated by \r."""
        logs = self._read_bt_com_payloads()
        data_parts = [l for l in logs if not l.startswith("OK+")]

        self._rx_buffer += "".join(data_parts)
        
        # 你的 Arduino println() 會送出 \r\n
        # 只要看見 \r，就代表一句完整的話結束了
        if '\r' in self._rx_buffer:
            line, self._rx_buffer = self._rx_buffer.split('\r', 1)
            
            # 安全機制：移除殘留的 \n，確保輸出乾淨的字串，而不使用 strip() 以免誤刪你想要的空白
            return line.replace('\n', '')
            
        return None

    def send(self, text):
        """Sends data to be forwarded to HM-10 via GATT."""
        self.ser.write(text.encode('utf-8'))