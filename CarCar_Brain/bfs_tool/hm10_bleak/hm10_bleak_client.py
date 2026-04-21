import asyncio
from bleak import BleakClient, BleakScanner

class HM10BleakClient:
    def __init__(self, target_name=None, target_address=None):
        self.target_name = target_name
        self.address = target_address
        self.client = None
        self.CHAR_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb"
        self._rx_buffer = ""
        # The usable payload per packet for HM-10/CC2541
        self.MTU_PAYLOAD = 20 

    async def connect(self):
        def device_filter(device, adv):
            name_match = (self.target_name is None) or (device.name == self.target_name)
            addr_match = (self.address is None) or (device.address.upper() == self.address.upper())
            return name_match and addr_match

        print(f"Scanning for {self.target_name or 'device'}...")
        device = await BleakScanner.find_device_by_filter(device_filter, timeout=10.0)

        if not device:
            print("❌ Device not found.")
            return False
        
        self.client = BleakClient(device)
        await self.client.connect()
        await self.client.start_notify(self.CHAR_UUID, self._notification_handler)
        print(f"✅ Connected to {device.name}")
        return self.client.is_connected

    def _notification_handler(self, sender, data):
        self._rx_buffer += data.decode('utf-8', errors='ignore')

    def listen(self):
        data = self._rx_buffer
        self._rx_buffer = ""
        return data

    async def send(self, message):
        """Sends data in 20-byte chunks to respect BLE MTU constraints."""
        if not self.client or not self.client.is_connected:
            return

        data = message.encode('utf-8')
        
        # Slicing the data into MTU-sized chunks
        for i in range(0, len(data), self.MTU_PAYLOAD):
            chunk = data[i:i + self.MTU_PAYLOAD]
            await self.client.write_gatt_char(
                self.CHAR_UUID, 
                chunk, 
                response=False 
            )
            # Short delay to allow the HM-10/Arduino to process the UART buffer
            # Without this, the HM-10 might drop packets during high-speed UART transfer
            await asyncio.sleep(0.02) 

    async def disconnect(self):
        if self.client:
            await self.client.disconnect()
            print("\nDisconnected.")