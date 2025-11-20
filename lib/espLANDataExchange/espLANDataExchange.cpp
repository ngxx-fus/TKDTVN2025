#include "../../include/projectConfig.h"
#if (LAN_DATA_EXCHANGE_EN == 1)

#include "espLANDataExchange.h"

WiFiUDP eldeUdp;
WiFiClient eldeTcp;

/// Buffer for RX UDP
#ifndef ELDE_RX_BUF_SIZE
#define ELDE_RX_BUF_SIZE 255
#endif
uint8_t eldeUdpRxBuf[ELDE_RX_BUF_SIZE];
int eldeUdpRxLen = 0;
bool eldeUdpHasData = false;

/// Static buffer to store Self IP string persistently
static char _selfIpStrBuf[16] = "0.0.0.0";

/// Define thisESP default configuration (Can be modified in setup() before init)
espLANHost_t thisESP = {
    .ip = _selfIpStrBuf,      ///< Will point to static buffer updated by eldeUpdateSelfIP
    .hostName = "ESP32_CLIENT",
    .port = {
        .udp = 4210,          ///< Default Listening UDP Port
        .tcp = 4211           ///< Default Listening TCP Port (if server mode used)
    }
};

#if (FIREBASE_SYNC_EN == 0)
    void wfInit(){
        /// Define wait intervals (microseconds)
        const int64_t SHORT_WAIT   = 2000000;   /// 2s
        const int64_t MEDIUM_WAIT  = 60000000;  /// 1 min
        const int64_t LONG_WAIT    = 600000000; /// 10 min

        int attemptCount = 0; 
        int64_t waitTimeUs = 0;
        espSoftTimer_t wfTimer;

        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        /// Loop until connected
        while(WiFi.status() != WL_CONNECTED){
            attemptCount++;
            
            /// Calculate backoff time based on attempt count
            if(attemptCount < 10){
                waitTimeUs = SHORT_WAIT;    // Retries 1-10: 2s
            } else if(attemptCount < 20){
                waitTimeUs = MEDIUM_WAIT;   // Retries 11-20: 1 min
            } else {
                waitTimeUs = LONG_WAIT;     // Retries 20+: 10 min
            }

            __sys_log("[wfInit] Connecting to Wi-fi... (Attempt: %d, Wait: %ds)", 
                    attemptCount, (int)(waitTimeUs/1000000));
            
            /// Initialize timer
            espSoftTimerInit(&wfTimer, waitTimeUs);

            /// Safe Wait: Yields to OS while waiting
            __EST_WAIT_EXEC(&wfTimer, vTaskDelay(pdMS_TO_TICKS(100)));
            
            /// Hard Retry: Re-trigger connection logic if stuck too long
            if (attemptCount % 20 == 0) {
                WiFi.disconnect();
                WiFi.reconnect();
            }
        }
        __sys_log("[wfInit] Connected to Wi-fi!");
        __sys_log("[wfInit] IP Address: %s", WiFi.localIP().toString().c_str());
    }
#endif /// (FIREBASE_SYNC_EN == 0)

/// Update thisESP.ip with current WiFi local IP
void eldeUpdateSelfIP() {
    if (WiFi.status() == WL_CONNECTED) {
        String ip = WiFi.localIP().toString();
        // Copy string to static buffer to ensure persistence
        strncpy(_selfIpStrBuf, ip.c_str(), sizeof(_selfIpStrBuf) - 1);
        _selfIpStrBuf[sizeof(_selfIpStrBuf) - 1] = '\0'; // Ensure null-terminate
    } else {
        __sys_err("[ELDE] UpdateIP Failed: WiFi Disconnected");
    }
}

/// Initialize UDP Listener based on thisESP configuration
def eldeInit() {
    if (WiFi.status() != WL_CONNECTED) {
        __sys_err("[ELDE] Init Failed: WiFi Disconnected");
        return STATUS_ERR;
    }

    // Update IP info
    eldeUpdateSelfIP();
    
    // Start listening on My UDP Port
    if (eldeUdp.begin(thisESP.port.udp)) {
        return STATUS_OKE;
    }
    
    __sys_err("[ELDE] UDP Begin Failed on Port: %d", thisESP.port.udp);
    return STATUS_ERR;
}

/// Send UDP Byte to Target
def espUDPSendByte(const espLANHost_t* target, uint8_t data) {
    if (target == NULL) {
        __sys_err("[ELDE] UDP Send Failed: Target NULL");
        return STATUS_ERR;
    }

    if (eldeUdp.beginPacket(target->ip, target->port.udp)) {
        eldeUdp.write(data);
        if (eldeUdp.endPacket()) {
            return STATUS_OKE;
        }
    }
    
    __sys_err("[ELDE] UDP Send Byte Failed -> %s:%d", target->ip, target->port.udp);
    return STATUS_ERR;
}

/// Send UDP Array to Target
def espUDPSendByteArr(const espLANHost_t* target, const uint8_t* data, size_t len) {
    if (target == NULL) {
        __sys_err("[ELDE] UDP SendArr Failed: Target NULL");
        return STATUS_ERR;
    }

    if (eldeUdp.beginPacket(target->ip, target->port.udp)) {
        eldeUdp.write(data, len);
        if (eldeUdp.endPacket()) {
            return STATUS_OKE;
        }
    }

    __sys_err("[ELDE] UDP Send Arr Failed -> %s:%d", target->ip, target->port.udp);
    return STATUS_ERR;
}

/// Send TCP Byte to Target
def espTCPSendByte(const espLANHost_t* target, uint8_t data) {
    if (target == NULL) {
        __sys_err("[ELDE] TCP Send Failed: Target NULL");
        return STATUS_ERR;
    }

    // Check if connected. If we switch targets, we might need to handle disconnects logic here or outside.
    if (!eldeTcp.connected()) {
        if (!eldeTcp.connect(target->ip, target->port.tcp)) {
            __sys_err("[ELDE] TCP Connect Failed -> %s:%d", target->ip, target->port.tcp);
            return STATUS_ERR;
        }
    }
    
    if (eldeTcp.write(data) > 0) {
        return STATUS_OKE;
    }

    __sys_err("[ELDE] TCP Write Byte Failed");
    return STATUS_ERR;
}

/// Send TCP Array to Target
def espTCPSendByteArr(const espLANHost_t* target, const uint8_t* data, size_t len) {
    if (target == NULL) {
        __sys_err("[ELDE] TCP SendArr Failed: Target NULL");
        return STATUS_ERR;
    }

    if (!eldeTcp.connected()) {
        if (!eldeTcp.connect(target->ip, target->port.tcp)) {
            __sys_err("[ELDE] TCP Connect Failed -> %s:%d", target->ip, target->port.tcp);
            return STATUS_ERR;
        }
    }

    if (eldeTcp.write(data, len) == len) {
        return STATUS_OKE;
    }
    
    __sys_err("[ELDE] TCP Write Arr Failed");
    return STATUS_ERR;
}

/// Poll Incoming UDP Data
void eldePoll() {
    int packetSize = eldeUdp.parsePacket();
    if (packetSize > 0) {
        // Limit read size to buffer size
        int len = packetSize > ELDE_RX_BUF_SIZE ? ELDE_RX_BUF_SIZE : packetSize;
        
        eldeUdp.read(eldeUdpRxBuf, len);
        eldeUdpRxLen = len;
        eldeUdpHasData = true; // Set flag
    }
}

/// Receive UDP Byte
def espUDPReceiveByte(uint8_t* outData) {
    if (eldeUdpHasData && eldeUdpRxLen > 0) {
        *outData = eldeUdpRxBuf[0];
        // Note: Does not clear flag, allowing ByteArr read of same packet
        return STATUS_OKE;
    }
    return STATUS_ERR;
}

/// Receive UDP Array
def espUDPReceiveByteArr(uint8_t* buffer, size_t maxLen) {
    if (eldeUdpHasData && eldeUdpRxLen > 0) {
        size_t copyLen = (eldeUdpRxLen < maxLen) ? eldeUdpRxLen : maxLen;
        memcpy(buffer, eldeUdpRxBuf, copyLen);
        
        eldeUdpHasData = false; // Clear flag after reading full packet
        return (def)copyLen;
    }
    return STATUS_ERR;
}

/// Receive TCP Byte
def espTCPReceiveByte(uint8_t* outData) {
    if (eldeTcp.connected() && eldeTcp.available()) {
        *outData = eldeTcp.read();
        return STATUS_OKE;
    }
    return STATUS_ERR;
}

/// Receive TCP Array
def espTCPReceiveByteArr(uint8_t* buffer, size_t maxLen) {
    if (eldeTcp.connected() && eldeTcp.available()) {
        int len = eldeTcp.read(buffer, maxLen);
        return (def)len;
    }
    return STATUS_ERR;
}

#endif /// (LAN_DATA_EXCHANGE_EN == 1)