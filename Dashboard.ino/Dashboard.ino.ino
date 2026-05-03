#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>

// --- PIN DEFINITIONS ---
#define NEO_PIN     19
#define BUZZ_PIN    18
#define SDA_PIN     21
#define SCL_PIN     22
#define NUM_LEDS    16 

// --- HARDWARE OBJECTS ---
Adafruit_NeoPixel ring(NUM_LEDS, NEO_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SH1106G display = Adafruit_SH1106G(128, 64, &Wire, -1);

// --- DATA STRUCTURE (Must exactly match Leaf Node) ---
typedef struct struct_message {
    int nodeID;
    float busVoltage; 
    float current_mA;
    float power_mW;
} struct_message;

struct_message liveData;

// --- STATE & TIMING VARIABLES ---
unsigned long lastRecvTime = 0;
unsigned long lastOledUpdate = 0;
unsigned long lastBlink = 0;
bool blinkIsOn = false;
const unsigned long TIMEOUT_MS = 3000; // 3 seconds offline timeout

// --- THEFT DETECTION LIMIT ---
const float THEFT_LIMIT_MW = 220.0; // Hardcoded fixed threshold

// --- ESP-NOW CALLBACK ---
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  memcpy(&liveData, incomingData, sizeof(liveData));
  lastRecvTime = millis(); 
}

void setup() {
  Serial.begin(115200);
  
  pinMode(BUZZ_PIN, OUTPUT);
  ring.begin();
  
  // --- STRICT LOW BRIGHTNESS LOCK ---
  ring.setBrightness(15); 
  ring.show();

  // THE RAINBOW BOOT RITUAL
  for(int i = 0; i < 3; i++) {
    tone(BUZZ_PIN, 2000 + (i * 500), 150);
    for(int j=0; j<256; j+=85) {
      for(int k=0; k<NUM_LEDS; k++) {
        ring.setPixelColor(k, ring.ColorHSV((j + (k*85)) * 256, 255, 255));
      }
      ring.show();
      delay(50); 
    }
  }
  tone(BUZZ_PIN, 3000, 300); // Final Confirmation Tone

  // Initialize Display
  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(0x3C, true);
  display.clearDisplay();
  display.display();

  // Initialize ESP-NOW
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Dashboard Active. Waiting for INA219 Node...");
}

void loop() {
  unsigned long now = millis();
  bool isConnected = (lastRecvTime != 0) && (now - lastRecvTime < TIMEOUT_MS);

  // --- DETERMINE CURRENT STATE ---
  int currentState = 1; // Default to Offline
  
  if (isConnected) {
    // Check for Bijli Chori against the fixed limit
    if (liveData.power_mW > THEFT_LIMIT_MW) {
      currentState = 3; // THEFT ALERT
    } else {
      currentState = 2; // NORMAL
    }
  } else {
    currentState = 1; // OFFLINE
  }

  // --- EXECUTE STATE LOGIC ---
  switch (currentState) {
    
    // ==========================================
    // STATE 1: OFFLINE
    // ==========================================
    case 1:
      for(int i=0; i<NUM_LEDS; i++) ring.setPixelColor(i, ring.Color(255, 0, 0));
      ring.show();

      if (now - lastOledUpdate > 500) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(20, 0); display.println("POWER SENTINEL");
        display.drawFastHLine(0, 12, 128, SH110X_WHITE);
        
        display.setTextSize(2);
        display.setCursor(25, 30); display.println("OFFLINE");
        display.display();
        lastOledUpdate = now;
      }
      break;

    // ==========================================
    // STATE 2: NORMAL OPERATION (Live Data)
    // ==========================================
    case 2:
      {
        uint32_t firstPixelHue = (now * 2) % 65536; 
        for(int i = 0; i < NUM_LEDS; i++) {
          uint32_t pixelHue = firstPixelHue + (i * 65536L / NUM_LEDS);
          ring.setPixelColor(i, ring.ColorHSV(pixelHue, 255, 255));
        }
        ring.show();
      }

      if (now - lastOledUpdate > 500) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(20, 0); display.println("POWER SENTINEL");
        display.drawFastHLine(0, 12, 128, SH110X_WHITE);

        display.setCursor(0, 22); display.print("Line V: "); display.print(liveData.busVoltage, 2); display.println(" V");
        display.setCursor(0, 37); display.print("Load I: "); display.print(liveData.current_mA, 1); display.println(" mA");
        display.setCursor(0, 52); display.print("Usage P:"); display.print(liveData.power_mW, 1); display.println(" mW");
        display.display();
        
        lastOledUpdate = now;
      }
      break;

    // ==========================================
    // STATE 3: THEFT ALERT (Clean "Bijli Chori" UI)
    // ==========================================
    case 3:
      // LEDs & Buzzer: Blinking Red
      if (now - lastBlink > 250) {
        blinkIsOn = !blinkIsOn;
        lastBlink = now;
        
        if (blinkIsOn) {
          for(int i=0; i<NUM_LEDS; i++) ring.setPixelColor(i, ring.Color(255, 0, 0));
          tone(BUZZ_PIN, 1200, 100); 
        } else {
          for(int i=0; i<NUM_LEDS; i++) ring.setPixelColor(i, 0); 
        }
        ring.show();
      }

      // OLED: Massive Text + Usage Power
      if (now - lastOledUpdate > 500) {
        display.clearDisplay();
        display.setTextColor(SH110X_WHITE);
        
        // Large "BIJLI CHORI" text, roughly centered
        display.setTextSize(2);
        display.setCursor(35, 5);  
        display.println("BIJLI");
        display.setCursor(28, 25); 
        display.println("CHORI!!");
        
        // Display the actual power usage causing the alarm
        display.setTextSize(1);
        display.setCursor(15, 50); 
        display.print("Usage P: "); 
        display.print(liveData.power_mW, 1); 
        display.println(" mW");
        
        display.display();
        lastOledUpdate = now;
      }
      break;
  }
}