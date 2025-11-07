#include "FirebaseWrap.h"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig fbconfig;

// ... (Các hàm wfIsConnected, wfInit, fbInit của bạn giữ nguyên) ...

DEFAULT_RETURN_STATUS wfIsConnected() {
  return (WiFi.status() == WL_CONNECTED) ? STATUS_OKE : STATUS_ERR;
}

void wfInit() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  do {
    __sys_log("[wfInit] Connecting WiFi...");
    delay(1000);
  } while (WiFi.status() != WL_CONNECTED);
  __sys_log("[wfInit] WiFi connected. IP: %s", WiFi.localIP().toString().c_str());
}

void fbInit() {
  __entry("fbInit()");

  // Đoạn này của bạn đã gán đúng, không bị nhầm như comment
  fbconfig.api_key = FB_API_KEY;
  fbconfig.database_url = FBRTDB_URL;

  auth.user.email = FB_USER_EMAIL;
  auth.user.password = FB_USER_PASSWORD;

  Firebase.reconnectWiFi(true);
  Firebase.begin(&fbconfig, &auth);

  while (!Firebase.ready()) {
    __sys_log("[fbInit] Waiting for Firebase.ready()...");
    delay(200);
  }
  __sys_log("[fbInit] Firebase ready!");
  __exit("fbInit()");
}


// *** GỢI Ý CẢI TIẾN ***
// Dùng setFloat() thay vì setString() để lưu dữ liệu dạng số
DEFAULT_RETURN_STATUS fbUploadMPU6050Data() {
  if (wfIsConnected() != STATUS_OKE) return STATUS_ERR;

  // Giả sử mpuData.ax, ay, az... là kiểu float
  // Nếu chúng là int, bạn có thể dùng setInt()
  bool ok = true;
  ok &= Firebase.RTDB.setFloat(&fbdo, FBRTDB_MPU6050_PATH "/ax", mpuData.ax);
  ok &= Firebase.RTDB.setFloat(&fbdo, FBRTDB_MPU6050_PATH "/ay", mpuData.ay);
  ok &= Firebase.RTDB.setFloat(&fbdo, FBRTDB_MPU6050_PATH "/az", mpuData.az);
  ok &= Firebase.RTDB.setFloat(&fbdo, FBRTDB_MPU6050_PATH "/gx", mpuData.gx);
  ok &= Firebase.RTDB.setFloat(&fbdo, FBRTDB_MPU6050_PATH "/gy", mpuData.gy);
  ok &= Firebase.RTDB.setFloat(&fbdo, FBRTDB_MPU6050_PATH "/gz", mpuData.gz);

  if (!ok) {
    __sys_err("[fbUploadMPU6050Data] RTDB write failed: %s",
              fbdo.errorReason().c_str());
    return STATUS_ERR;
  }
  return STATUS_OKE;
}