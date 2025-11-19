import socket
import struct
import sys
import time

# --- Configuration ---
UDP_IP   = "0.0.0.0"    # Listen on all interfaces
UDP_PORT = 4210         # Port to listen on
ESP_PORT = 4210         # Port to send Control commands back to ESP32
BUFFER_SIZE = 1024

# --- Protocol Header/Footer Definitions ---
# 1. GPS (ATGM336H)
ATGM_START  = b"ATGM336H_START"
ATGM_STOP   = b"ATGM336H_STOP"

# 2. IMU (MPU6050)
MPU_START   = b"MPU6050_START"
MPU_STOP    = b"MPU6050_STOP"

# 3. Ultrasonic (HCSR04)
HC_START    = b"HCSR04_START"
HC_STOP     = b"HCSR04_STOP"

# 4. Error Reporting (System Errors)
ERR_START   = b"SYS_ERR_START"
ERR_STOP    = b"SYS_ERR_STOP"

# 5. Control (PC -> ESP32) [Reserved]
CTRL_START  = b"CTRL_START"
CTRL_STOP   = b"CTRL_STOP"

# --- FRAME FILTER ---
# Add tags here to ACCEPT them. Remove to IGNORE.
FRAME_FILTER = [
    ATGM_START,
    MPU_START,
    HC_START,
    ERR_START
]

# --- Struct Definitions (Little Endian <) ---

# 1. ATGM336H Data (20 bytes)
# C: uint8 h, m, s, d, mo; uint16 y; float lat, lon, spd; uint8 sats
ATGM_FMT = "<BBBBBHfffB" 

# 2. MPU6050 Data (12 bytes)
# C: int16 ax, ay, az, gx, gy, gz
MPU_FMT  = "<hhhhhh"

# 3. HCSR04 Data (8 bytes)
# C: int16 front, right, back, left
HC_FMT   = "<hhhh"

# 4. Error Data (Example: 5 bytes)
# C: uint32_t timestamp, uint8_t error_code
ERR_FMT  = "<IB" 

# 5. Control Data (Example: 5 bytes)
# C: uint8_t command_id, int32_t value
CTRL_FMT = "<Bi"

# --- Protocol Registry ---
PROTOCOLS = {
    ATGM_START: {
        "stop": ATGM_STOP, "fmt": ATGM_FMT, "size": struct.calcsize(ATGM_FMT), "name": "GPS"
    },
    MPU_START: {
        "stop": MPU_STOP,  "fmt": MPU_FMT,  "size": struct.calcsize(MPU_FMT),  "name": "IMU"
    },
    HC_START: {
        "stop": HC_STOP,   "fmt": HC_FMT,   "size": struct.calcsize(HC_FMT),   "name": "US "
    },
    ERR_START: {
        "stop": ERR_STOP,  "fmt": ERR_FMT,  "size": struct.calcsize(ERR_FMT),  "name": "ERR"
    }
}

# --- Handlers ---

def handle_gps(unpacked):
    h, m, s, d, mo, y, lat, lon, spd, sats = unpacked
    print(f"[GPS] {h:02}:{m:02}:{s:02} | {lat:.6f}, {lon:.6f} | {spd:.2f}km/h | Sats:{sats}")

def handle_mpu(unpacked):
    ax, ay, az, gx, gy, gz = unpacked
    # Scale factors usually needed here depending on sensor settings
    print(f"[IMU] Acc: {ax:5} {ay:5} {az:5} | Gyr: {gx:5} {gy:5} {gz:5}")

def handle_hc(unpacked):
    f, r, b, l = unpacked
    print(f"[US ] Dist: F={f}cm R={r}cm B={b}cm L={l}cm")

def handle_err(unpacked):
    ts, code = unpacked
    print(f"[ERR] System Error Code: {code} at Timestamp: {ts}")

# --- Control Function (PC -> ESP32) ---
def send_control_command(sock, ip, cmd_id, value):
    """
    Sends a control frame back to ESP32.
    Struct: StartTag + CommandID(u8) + Value(i32) + StopTag
    """
    payload = struct.pack(CTRL_FMT, cmd_id, value)
    packet = CTRL_START + payload + CTRL_STOP
    sock.sendto(packet, (ip, ESP_PORT))
    print(f"[CTRL] Sent CMD:{cmd_id} VAL:{value} to {ip}")

# --- Main Loop ---
def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.bind((UDP_IP, UDP_PORT))
        print(f"[INFO] Listening on UDP {UDP_PORT}...")
        print(f"[INFO] Active Filters: {[tag.decode() for tag in FRAME_FILTER]}")
    except Exception as e:
        print(f"[FATAL] Socket bind failed: {e}")
        sys.exit(1)

    while True:
        try:
            data, addr = sock.recvfrom(BUFFER_SIZE)
            
            # 1. Identify Protocol
            detected_tag = None
            for tag in PROTOCOLS.keys():
                if tag in data:
                    detected_tag = tag
                    break
            
            # Case A: Unknown Frame
            if detected_tag is None:
                # print(f"[UNK] Unknown data from {addr}") # Enable to debug noise
                continue

            # Case B: Filtered Out
            if detected_tag not in FRAME_FILTER:
                continue

            # Case C: Valid Header Found
            proto = PROTOCOLS[detected_tag]
            start_idx = data.find(detected_tag)

            # Trim garbage
            if start_idx != 0:
                data = data[start_idx:]

            # Offsets
            p_start = len(detected_tag)
            f_start = p_start + proto["size"]
            expected_len = f_start + len(proto["stop"])

            # Check Length
            if len(data) < expected_len:
                print(f"[ERR] {proto['name']} Incomplete. Got {len(data)}, need {expected_len}")
                continue

            # Verify Footer
            received_footer = data[f_start : f_start + len(proto["stop"])]
            if received_footer != proto["stop"]:
                print(f"[ERR] {proto['name']} Footer mismatch!")
                continue

            # Unpack & Route
            raw = data[p_start : f_start]
            try:
                unpacked = struct.unpack(proto["fmt"], raw)
                
                if detected_tag == ATGM_START: handle_gps(unpacked)
                elif detected_tag == MPU_START: handle_mpu(unpacked)
                elif detected_tag == HC_START: handle_hc(unpacked)
                elif detected_tag == ERR_START: handle_err(unpacked)

                # Example: Send a command back every time we get a specific packet
                # if detected_tag == HC_START:
                #     send_control_command(sock, addr[0], 1, 100)

            except struct.error as e:
                print(f"[ERR] Unpack failed: {e}")

        except KeyboardInterrupt:
            print("\n[INFO] Stopping...")
            break
        except Exception as e:
            print(f"[EXC] {e}")

if __name__ == "__main__":
    main()