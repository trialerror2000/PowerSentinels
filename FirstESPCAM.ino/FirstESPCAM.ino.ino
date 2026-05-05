#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// ===== WiFi AP =====
const char* ssid = "ESP32-CAM";
const char* password = "12345678";

// ===== Server =====
WebServer server(80);

// ===== AI THINKER PINS =====
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// ===== HTML PAGE =====
const char* index_html = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <title>ESP32 CAM</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      body { background:#111; color:#fff; text-align:center; }
      img { width: 100%; max-width: 600px; border: 2px solid #333; }
    </style>
  </head>
  <body>
    <h2>ESP32-CAM Live Feed</h2>
    <img src="/stream">
  </body>
</html>
)rawliteral";

// ===== ROOT =====
void handleRoot() {
  server.send(200, "text/html", index_html);
}

// ===== STREAM =====
void handleStream() {
  WiFiClient client = server.client();

  String response = 
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  client.print(response);

  while (client.connected()) {
    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) continue;

    client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    esp_camera_fb_return(fb);

    delay(30); // prevent overflow
  }
}

// ===== CAMERA SETUP =====
void startCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // 🔥 PSRAM OPTIMIZED
  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera failed: 0x%x\n", err);
    while (true);
  }

  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == 0x3660) {
    s->set_vflip(s, 1);
  }
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  startCamera();

  WiFi.softAP(ssid, password);
  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/stream", handleStream);

  server.begin();
}

// ===== LOOP =====
void loop() {
  server.handleClient();
}