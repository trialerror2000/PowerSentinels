// {0xC0, 0x5D, 0x89, 0xB1, 0x1C, 0x24}

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_NeoPixel.h>

// --- REPLACE WITH YOUR ESP32 MAC ADDRESS ---
uint8_t parentAddress[] = {0xC0, 0x5D, 0x89, 0xB1, 0x1C, 0x24};

// --- PIN DEFINITIONS ---
#define NEO_PIN D4 // Wemos Pin D4
#define NUM_LEDS 1

// --- HARDWARE OBJECTS ---
Adafruit_NeoPixel pixel(NUM_LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_INA219 ina219;

// --- DATA STRUCTURE (Must match Parent Node) ---
typedef struct struct_message {
    int nodeID;
    float busVoltage; 
    float current_mA;
    float power_mW;
} struct_message;

struct_message myData;

// --- STATE VARIABLES ---
bool isConnected = false;
unsigned long lastBlink = 0;
bool ledState = false;

// --- ESP-NOW CALLBACK ---
// This tells us if the parent node successfully received our data
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if (sendStatus == 0) {
    isConnected = true;
  } else {
    isConnected = false;
  }
}

void setup() {
  Serial.begin(115200);

  // 1. Initialize NeoPixel
  pixel.begin();
  pixel.setBrightness(50); // Safe brightness for a single LED
  pixel.setPixelColor(0, pixel.Color(255, 0, 0)); // Start Red
  pixel.show();

  // 2. Initialize INA219
  Wire.begin(); // Uses D1 (SCL) and D2 (SDA) by default on Wemos
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip. Check wiring!");
    while (1) { delay(10); } // Halt if sensor is missing
  }
  // To use a slightly lower 32V, 1A range (better precision for small loads):
  // ina219.setCalibration_32V_1A();
  Serial.println("INA219 Active.");

  // 3. Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // 4. Register Parent Node
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  esp_now_add_peer(parentAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
  
  // Set Node ID
  myData.nodeID = 1; 
}

void loop() {
  unsigned long now = millis();

  // --- 1. READ SENSOR DATA ---
  myData.busVoltage = ina219.getBusVoltage_V();
  myData.current_mA = ina219.getCurrent_mA();
  myData.power_mW = ina219.getPower_mW();

  // --- 2. SEND DATA ---
  esp_now_send(parentAddress, (uint8_t *) &myData, sizeof(myData));

  // --- 3. LED STATUS INDICATOR ---
  if (isConnected) {
    // Solid Green when actively communicating
    pixel.setPixelColor(0, pixel.Color(0, 255, 0));
    pixel.show();
  } else {
    // Blinking Red when parent is off or out of range
    if (now - lastBlink > 250) { // Blink every 250ms
      ledState = !ledState;
      lastBlink = now;
      if (ledState) {
        pixel.setPixelColor(0, pixel.Color(255, 0, 0));
      } else {
        pixel.setPixelColor(0, pixel.Color(0, 0, 0));
      }
      pixel.show();
    }
  }

  // Send data rapidly to ensure the Parent Node detects theft instantly
  delay(100); 
}