import socket
import struct
import threading
import time
import sys

# /// -------------------------------------------------------------------------
# /// CONFIGURATION & CONSTANTS
# /// -------------------------------------------------------------------------

# /// UDP Configuration
UDP_BROADCAST_IP = "255.255.255.255"
UDP_IP           = "0.0.0.0"       # /// Listen on all interfaces

# /// List of ports to listen on simultaneously
PORT_LISTEN_LIST = [2578, 3546] 

ESP_PORT         = 2578            # /// Destination port for ESP32
BUFFER_SIZE      = 1024            # /// Buffer size for socket

# /// Frame Literals
# /// Frame: <FRAME_BEGIN> <ID> <DATA...> <CRC_TAG> <CRC_HI> <CRC_LO> <FRAME_END>
FRAME_BEGIN      = b"FRAME_BEGIN"  # /// Hex: 4652414d455f424547494e
FRAME_END        = b"FRAME_END"    # /// Hex: 4652414d455f454e44
CRC_TAG          = b"CRC"          # /// Hex: 435243

# /// Protocol IDs (Must match MCU definitions)
ID_CTRL          = 40
ID_MPU6050       = 41
ID_HCSR04        = 42
ID_ATGM336H      = 43

# /// -------------------------------------------------------------------------
# /// CRC ALGORITHM
# /// -------------------------------------------------------------------------

# /// Compute CRC16 (CCITT-FALSE/0x1021) for data integrity check
# /// @param data: Bytes to calculate CRC on (ID + Payload)
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
class MPU6050Data(SensorData):
    def __init__(self):
        super().__init__()
        self.ax = 0; self.ay = 0; self.az = 0
        self.gx = 0; self.gy = 0; self.gz = 0
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
class HCSR04Data(SensorData):
    def __init__(self):
        super().__init__()
        self.dist_f = 0; self.dist_r = 0
        self.dist_b = 0; self.dist_l = 0
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
class ATGM336HData(SensorData):
    def __init__(self):
        super().__init__()
        self.hour = 0; self.min = 0; self.sec = 0
        self.day = 0; self.mon = 0; self.year = 0
        self.lat = 0.0; self.lon = 0.0; self.speed = 0.0
        self.sats = 0
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

# /// Class representing General/Unknown Data
# /// Handles any ID not explicitly defined above
class GeneralData(SensorData):
    def __init__(self):
        super().__init__()
        self.packet_id = 0
        self.data_size = 0
        self.raw_data = b""

    # /// Update the storage with ID and Raw Bytes
    # /// @param pid: The Protocol ID extracted from header
    # /// @param data: The raw payload bytes
    def update(self, pid: int, data: bytes):
        self.packet_id = pid
        self.raw_data = data
        self.data_size = len(data)
        self.updated = True
        
        # /// Convert to hex string for easy visualization
        hex_view = " ".join(f"{b:02X}" for b in self.raw_data)
        print(f"[GEN] ID:{self.packet_id} Size:{self.data_size} Payload:[{hex_view}]")

    # /// Get a specific byte at index i safely
    # /// @return: Byte value (int) or 0 if out of bounds
    def get_byte(self, index: int) -> int:
        if 0 <= index < self.data_size:
            return self.raw_data[index]
        return 0

# /// -------------------------------------------------------------------------
# /// GLOBAL STORAGE
# /// -------------------------------------------------------------------------

class GlobalStorage:
    def __init__(self):
        self.mpu = MPU6050Data()
        self.hcsr = HCSR04Data()
        self.gps = ATGM336HData()
        self.general = GeneralData() # /// Storage for unknown frames
        
        self.lock = threading.Lock()
        self.target_ip = None 

globals_data = GlobalStorage()

# /// -------------------------------------------------------------------------
# /// THREAD WORKERS
# /// -------------------------------------------------------------------------

# /// Worker for receiving and parsing UDP frames from a specific socket
class ReceiveThread(threading.Thread):
    def __init__(self, socket_obj, port_id):
        super().__init__()
        self.sock = socket_obj
        self.port_id = port_id
        self.running = True

    # /// Helpers for unpacking known types
    def UnpackMPU6050(self, payload):
        with globals_data.lock:
            globals_data.mpu.unpack(payload)

    def UnpackHCSR04(self, payload):
        with globals_data.lock:
            globals_data.hcsr.unpack(payload)

    def UnpackATGM336H(self, payload):
        with globals_data.lock:
            globals_data.gps.unpack(payload)
            
    # /// Helper for unpacking unknown types
    def UnpackGeneral(self, pid, payload):
        with globals_data.lock:
            globals_data.general.update(pid, payload)

    # /// Main execution loop for receiver
    def run(self):
        print(f"[RX-THREAD-{self.port_id}] Started listening on port {self.port_id}.")
        while self.running:
            try:
                data, addr = self.sock.recvfrom(BUFFER_SIZE)
                
                # /// Store Sender IP (Last known sender from any port)
                if globals_data.target_ip != addr[0]:
                    globals_data.target_ip = addr[0]
                    print(f"[RX-{self.port_id}] New Connection from {globals_data.target_ip}")

                # /// 1. Validate Frame Boundaries
                if not (data.startswith(FRAME_BEGIN) and data.endswith(FRAME_END)):
                    continue 

                # /// 2. Strip Header and Footer
                # /// Frame: [BEGIN] [ID+Payload] [CRC_TAG] [HI] [LO] [END]
                inner_content = data[len(FRAME_BEGIN) : -len(FRAME_END)]
                if len(inner_content) < 6:
                    continue

                # /// 3. Separate Payload and CRC Section
                # /// CRC Section = "CRC" (3 bytes) + Val (2 bytes) = 5 bytes
                crc_block_len = len(CRC_TAG) + 2 
                payload = inner_content[: -crc_block_len] 
                crc_section = inner_content[-crc_block_len:]

                # /// 4. Check CRC Tag Presence
                if not crc_section.startswith(CRC_TAG):
                    continue

                # /// 5. Verify CRC Integrity
                rx_crc_bytes = crc_section[len(CRC_TAG):]
                rx_crc_val = (rx_crc_bytes[0] << 8) | rx_crc_bytes[1]
                
                # /// Calculate CRC on [ID + Data]
                calc_crc = computeCRC16_CCITT_FALSE(payload)

                if rx_crc_val != calc_crc:
                    print(f"[RX-{self.port_id}] CRC Mismatch")
                    continue 

                # /// 6. Route based on ID
                # /// payload[0] is ID, payload[1:] is Data
                packet_id = payload[0]
                packet_data = payload[1:] 

                if packet_id == ID_MPU6050:
                    self.UnpackMPU6050(packet_data)
                elif packet_id == ID_HCSR04:
                    self.UnpackHCSR04(packet_data)
                elif packet_id == ID_ATGM336H:
                    self.UnpackATGM336H(packet_data)
                else:
                    # /// Route unknown ID to GeneralData
                    self.UnpackGeneral(packet_id, packet_data)

            except Exception as e:
                # /// Ignore socket closed errors on shutdown
                if self.running:
                    print(f"[RX-ERR-{self.port_id}] {e}")

    # /// Stop the thread safely
    def stop(self):
        self.running = False

# /// Worker for sending control commands
class SendThread(threading.Thread):
    def __init__(self, socket_obj):
        super().__init__()
        self.sock = socket_obj # /// Uses one of the bound sockets to send
        self.running = True

    # /// Construct a frame with CRC
    # /// Frame: BEGIN + ID + DATA + "CRC" + CRC16 + END
    def build_frame(self, cmd_id: int, data_bytes: bytes) -> bytes:
        payload = struct.pack("B", cmd_id) + data_bytes
        crc_val = computeCRC16_CCITT_FALSE(payload)
        crc_bytes = struct.pack(">H", crc_val) 
        return FRAME_BEGIN + payload + CRC_TAG + crc_bytes + FRAME_END

    # /// Main execution loop for sender
    def run(self):
        print("[TX-THREAD] Started.")
        while self.running:
            time.sleep(1.0) 

            # /// Just broadcast if we don't have a target yet
            target = globals_data.target_ip if globals_data.target_ip else UDP_BROADCAST_IP

            try:
                # /// Example: Send Ping/Keep-Alive
                cmd_id = ID_CTRL
                raw_data = bytes([0xFF, 0x00, 0xEE, 0x00])
                packet = self.build_frame(cmd_id, raw_data)
                
                # /// Send to the ESP port
                self.sock.sendto(packet, (target, ESP_PORT))
                
            except Exception as e:
                if self.running:
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
        self.sockets = []
        self.rx_threads = []
        self.tx_thread = None

        # /// Initialize Sockets for all ports in list
        self.init_sockets()

    # /// Create a socket for each port in PORT_LISTEN_LIST
    def init_sockets(self):
        print(f"/// Initializing Listener on ports: {PORT_LISTEN_LIST} ///")
        
        for port in PORT_LISTEN_LIST:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
                sock.bind((UDP_IP, port))
                
                # /// Add to managed list
                self.sockets.append(sock)
                
                # /// Create a dedicated RX thread for this socket
                rx_t = ReceiveThread(sock, port)
                self.rx_threads.append(rx_t)
                
            except Exception as e:
                print(f"[INIT-ERR] Failed to bind port {port}: {e}")

        # /// Assign the first successfully bound socket to the TX Thread
        # /// (Any bound socket can send data out)
        if len(self.sockets) > 0:
            self.tx_thread = SendThread(self.sockets[0])
        else:
            print("[CRITICAL] No sockets could be bound. Exiting.")
            sys.exit(1)

    # /// Start all tasks
    def start(self):
        # /// Start all RX threads
        for t in self.rx_threads:
            t.start()
        
        # /// Start TX thread
        # if self.tx_thread:
            # self.tx_thread.start()
        
        self.main_loop()

    # /// Main Thread: Event Loop / Supervisor
    def main_loop(self):
        try:
            while True:
                time.sleep(0.5)
        except KeyboardInterrupt:
            print("\n[INFO] Shutting down...")
            self.stop()

    # /// Cleanup and stop threads
    def stop(self):
        # /// Signal all threads to stop
        for t in self.rx_threads:
            t.stop()
        if self.tx_thread:
            self.tx_thread.stop()
        
        # /// Close all sockets to unblock recvfrom()
        print("[INFO] Closing sockets...")
        for s in self.sockets:
            s.close()
        
        # /// Wait for threads to finish
        for t in self.rx_threads:
            t.join()
        if self.tx_thread:
            self.tx_thread.join()
            
        print("[INFO] Bye.")

if __name__ == "__main__":
    app = CenterControlApp()
    app.start()