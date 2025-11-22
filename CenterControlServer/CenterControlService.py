import socket
import struct
import threading
import time
import sys

# /// -------------------------------------------------------------------------
# /// CONFIGURATION & CONSTANTS
# /// -------------------------------------------------------------------------

# /// UDP Configuration
UDP_BROADCAST_IP= "255.255.255.255"
UDP_IP          = "0.0.0.0"       # /// Listen on all interfaces
UDP_PORT        = 3546            # /// Port to listen on
ESP_PORT        = 2578            # /// Destination port for ESP32
BUFFER_SIZE     = 1024            # /// Buffer size for socket

# /// Frame Literals
FRAME_BEGIN     = b"FRAME_BEGIN"
FRAME_END       = b"FRAME_END"
CRC_TAG         = b"CRC"

# /// Protocol IDs (Must match MCU definitions)
ID_CTRL         = 40
ID_MPU6050      = 41
ID_HCSR04       = 42
ID_ATGM336H     = 43

# /// -------------------------------------------------------------------------
# /// CRC ALGORITHM
# /// -------------------------------------------------------------------------

# /// Compute CRC16 (CCITT-FALSE/0x1021) for data integrity check
# /// @param data: Bytes to calculate CRC on
# /// @return: 16-bit integer CRC value
def computeCRC16_CCITT_FALSE(data: bytes) -> int:
    crc = 0xFFFF
    poly = 0x1021
    
    for byte in data:
        crc ^= (byte << 8)
        for _ in range(8):
            if (crc & 0x8000):
                crc = (crc << 1) ^ poly
            else:
                crc = crc << 1
            crc &= 0xFFFF
    return crc

# /// -------------------------------------------------------------------------
# /// DATA STRUCTURES (CLASSES)
# /// -------------------------------------------------------------------------

# /// Base class for sensor data handling
class SensorData:
    def __init__(self):
        self.updated = False

# /// Class representing MPU6050 Data (Acc & Gyro)
# /// Structure corresponds to 6 int16_t values
class MPU6050Data(SensorData):
    def __init__(self):
        super().__init__()
        self.ax = 0; self.ay = 0; self.az = 0
        self.gx = 0; self.gy = 0; self.gz = 0
        # /// Format: 6 short (int16), Little Endian
        self.fmt = "<hhhhhh" 

    # /// Unpack raw bytes into class attributes
    def unpack(self, data: bytes):
        try:
            self.ax, self.ay, self.az, self.gx, self.gy, self.gz = struct.unpack(self.fmt, data)
            self.updated = True
            print(f"[MPU6050] Ax:{self.ax} Ay:{self.ay} Az:{self.az} | Gx:{self.gx} Gy:{self.gy} Gz:{self.gz}")
        except struct.error:
            print("[ERR] MPU6050 unpack failed size mismatch")

# /// Class representing HCSR04 Data (4 Ultrasonic sensors)
# /// Structure corresponds to 4 int16_t values
class HCSR04Data(SensorData):
    def __init__(self):
        super().__init__()
        self.dist_f = 0; self.dist_r = 0
        self.dist_b = 0; self.dist_l = 0
        # /// Format: 4 short (int16), Little Endian
        self.fmt = "<hhhh"

    # /// Unpack raw bytes into class attributes
    def unpack(self, data: bytes):
        try:
            self.dist_f, self.dist_r, self.dist_b, self.dist_l = struct.unpack(self.fmt, data)
            self.updated = True
            print(f"[HCSR04] Front:{self.dist_f} Right:{self.dist_r} Back:{self.dist_b} Left:{self.dist_l}")
        except struct.error:
            print("[ERR] HCSR04 unpack failed size mismatch")

# /// Class representing ATGM336H GPS Data
# /// Structure corresponds to packed struct: Time(5xB), Year(H), Lat/Lon/Spd(3xf), Sats(B)
class ATGM336HData(SensorData):
    def __init__(self):
        super().__init__()
        self.hour = 0; self.min = 0; self.sec = 0
        self.day = 0; self.mon = 0; self.year = 0
        self.lat = 0.0; self.lon = 0.0; self.speed = 0.0
        self.sats = 0
        # /// Format: 5xU8, 1xU16, 3xFloat, 1xU8 (Packed, Little Endian)
        self.fmt = "<BBBBBHfffB"

    # /// Unpack raw bytes into class attributes
    def unpack(self, data: bytes):
        try:
            (self.hour, self.min, self.sec, self.day, self.mon, self.year,
             self.lat, self.lon, self.speed, self.sats) = struct.unpack(self.fmt, data)
            self.updated = True
            print(f"[GPS] {self.hour}:{self.min}:{self.sec} | Loc: {self.lat:.5f}, {self.lon:.5f} | Sats: {self.sats}")
        except struct.error:
            print("[ERR] ATGM336H unpack failed size mismatch")

# /// -------------------------------------------------------------------------
# /// GLOBAL STORAGE
# /// -------------------------------------------------------------------------

class GlobalStorage:
    def __init__(self):
        self.mpu = MPU6050Data()
        self.hcsr = HCSR04Data()
        self.gps = ATGM336HData()
        self.lock = threading.Lock()
        self.target_ip = None # /// Store the IP of the sender to reply to

globals_data = GlobalStorage()

# /// -------------------------------------------------------------------------
# /// THREAD WORKERS
# /// -------------------------------------------------------------------------

# /// Worker for receiving and parsing UDP frames
class ReceiveThread(threading.Thread):
    def __init__(self, socket_obj):
        super().__init__()
        self.sock = socket_obj
        self.running = True

    # /// Helper: Parse MPU6050 packet
    def UnpackMPU6050(self, payload):
        with globals_data.lock:
            globals_data.mpu.unpack(payload)

    # /// Helper: Parse HCSR04 packet
    def UnpackHCSR04(self, payload):
        with globals_data.lock:
            globals_data.hcsr.unpack(payload)

    # /// Helper: Parse ATGM336H packet
    def UnpackATGM336H(self, payload):
        with globals_data.lock:
            globals_data.gps.unpack(payload)

    # /// Main execution loop for receiver
    def run(self):
        print("[RX-THREAD] Started.")
        while self.running:
            try:
                data, addr = self.sock.recvfrom(BUFFER_SIZE)
                
                # /// Store Sender IP for the SendThread to use
                if globals_data.target_ip != addr[0]:
                    globals_data.target_ip = addr[0]
                    print(f"[RX-THREAD] Connection from {globals_data.target_ip}")

                # /// 1. Validate Frame Boundaries
                if not (data.startswith(FRAME_BEGIN) and data.endswith(FRAME_END)):
                    continue # Discard noise

                # /// 2. Strip Header and Footer
                # Content: <ID><Data...><"CRC"><High><Low>
                inner_content = data[len(FRAME_BEGIN) : -len(FRAME_END)]

                # /// 3. Separate Payload and CRC Section
                # We expect the last 5 bytes to be: b'C', b'R', b'C', CRCHi, CRCLo
                # Length check: Must be at least ID(1) + CRC(3) + Val(2) = 6 bytes
                if len(inner_content) < 6:
                    continue

                # /// Extract Payload (ID + Data) and CRC fields
                # The CRC block (Tag + 2 bytes) is 5 bytes long at the end
                crc_block_len = len(CRC_TAG) + 2 
                payload = inner_content[: -crc_block_len] 
                crc_section = inner_content[-crc_block_len:]

                # /// 4. Check CRC Tag Presence
                if not crc_section.startswith(CRC_TAG):
                    print("[RX] Missing CRC Tag in frame")
                    continue

                # /// 5. Extract Receive CRC Value
                rx_crc_bytes = crc_section[len(CRC_TAG):]
                rx_crc_val = (rx_crc_bytes[0] << 8) | rx_crc_bytes[1]

                # /// 6. Calculate CRC on Payload
                calc_crc = computeCRC16_CCITT_FALSE(payload)

                if rx_crc_val != calc_crc:
                    print(f"[RX] CRC Mismatch! Rx:{hex(rx_crc_val)} vs Calc:{hex(calc_crc)}")
                    continue # Discard frame

                # /// 7. Route based on ID
                packet_id = payload[0]
                packet_data = payload[1:] # Rest is data

                if packet_id == ID_MPU6050:
                    self.UnpackMPU6050(packet_data)
                elif packet_id == ID_HCSR04:
                    self.UnpackHCSR04(packet_data)
                elif packet_id == ID_ATGM336H:
                    self.UnpackATGM336H(packet_data)
                else:
                    print(f"[RX] Unknown ID: {packet_id}")

            except Exception as e:
                print(f"[RX-ERR] {e}")

    # /// Stop the thread safely
    def stop(self):
        self.running = False

# /// Worker for sending control commands
class SendThread(threading.Thread):
    def __init__(self, socket_obj):
        super().__init__()
        self.sock = socket_obj
        self.running = True

    # /// Construct a frame with the specified ID and binary data
    # /// Frame: BEGIN + ID + DATA + "CRC" + CRC16 + END
    def build_frame(self, cmd_id: int, data_bytes: bytes) -> bytes:
        payload = struct.pack("B", cmd_id) + data_bytes
        crc_val = computeCRC16_CCITT_FALSE(payload)
        crc_bytes = struct.pack(">H", crc_val) # Big Endian for CRC High/Low manual handling
        
        frame = FRAME_BEGIN + payload + CRC_TAG + crc_bytes + FRAME_END
        return frame

    # /// Main execution loop for sender
    def run(self):
        print("[TX-THREAD] Started.")
        while self.running:
            time.sleep(1.0) # /// Send frequency: 1Hz

            if globals_data.target_ip is None:
                continue # Wait until we hear from ESP32

            try:
                # /// Req: Send ID=0, 4 bytes FF 00 EE 00
                cmd_id = ID_CTRL
                raw_data = bytes([0xFF, 0x00, 0xEE, 0x00])
                
                packet = self.build_frame(cmd_id, raw_data)
                
                # self.sock.sendto(packet, (globals_data.target_ip, ESP_PORT))
                self.sock.sendto(packet, (UDP_BROADCAST_IP, ESP_PORT))
                # print(f"[TX] Sent Ctrl Frame to {globals_data.target_ip}")
                print(f"[TX] Sent Ctrl Frame to {UDP_BROADCAST_IP}")
                
            except Exception as e:
                print(f"[TX-ERR] {e}")

    # /// Stop the thread safely
    def stop(self):
        self.running = False

# /// -------------------------------------------------------------------------
# /// MAIN APP
# /// -------------------------------------------------------------------------

# /// Main Application Class
class CenterControlApp:
    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
        self.sock.bind((UDP_IP, UDP_PORT))
        # /// Set timeout to allow thread to check stop condition occasionally
        # self.sock.settimeout(1.0) 
        
        self.rx_thread = ReceiveThread(self.sock)
        self.tx_thread = SendThread(self.sock)

    # /// Start all tasks
    def start(self):
        print(f"/// Center Control Service Started on UDP {UDP_PORT} ///")
        self.rx_thread.start()
        self.tx_thread.start()
        
        self.main_loop()

    # /// Main Thread: Event Loop / Supervisor
    def main_loop(self):
        try:
            while True:
                # /// Process events if necessary (GUI or Logic)
                # /// Currently just idle to keep main thread alive
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("\n[INFO] Shutting down...")
            self.stop()

    # /// Cleanup and stop threads
    def stop(self):
        self.rx_thread.stop()
        self.tx_thread.stop()
        
        # /// Close socket to force threads out of blocking calls
        self.sock.close()
        
        self.rx_thread.join()
        self.tx_thread.join()
        print("[INFO] Bye.")

if __name__ == "__main__":
    app = CenterControlApp()
    app.start()