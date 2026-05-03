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

// --- STATE & TIMING VARIABLES ---
int currentState = 2; // Start in State 2 (Normal mode)
unsigned long lastOledUpdate = 0;
unsigned long lastBlink = 0;
bool blinkIsOn = false;

// --- DATA VARIABLES ---
float testVoltage = 0;
float testCurrent = 0;
float testPower = 0;

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

  Wire.begin(SDA_PIN, SCL_PIN);
  display.begin(0x3C, true);
  display.clearDisplay();
  display.display();

  Serial.println("Send 1 (Offline), 2 (Normal), or 3 (Alert)");
}

void loop() {
  unsigned long now = millis();

  // --- CHECK FOR SERIAL COMMANDS ---
  if (Serial.available() > 0) {
    int input = Serial.parseInt();
    if (input >= 1 && input <= 3) {
      currentState = input;
      lastOledUpdate = 0; // Force immediate OLED refresh on state change
      Serial.print("Switched to State: ");
      Serial.println(currentState);
    }
  }

  // --- THE STATE MACHINE ---
  switch (currentState) {
    
    // ==========================================
    // STATE 1: OFFLINE
    // ==========================================
    case 1:
      // LEDs: Solid Red
      for(int i=0; i<NUM_LEDS; i++) ring.setPixelColor(i, ring.Color(255, 0, 0));
      ring.show();

      // OLED: Show Offline Status
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
    // STATE 2: NORMAL OPERATION
    // ==========================================
    case 2:
      // LEDs: Ultra-smooth, slow circular RGB effect
      {
        uint32_t firstPixelHue = (now * 2) % 65536; 
        for(int i = 0; i < NUM_LEDS; i++) {
          uint32_t pixelHue = firstPixelHue + (i * 65536L / NUM_LEDS);
          ring.setPixelColor(i, ring.ColorHSV(pixelHue, 255, 255));
        }
        ring.show();
      }

      // OLED: Generate Random Data every 1 second
      if (now - lastOledUpdate > 1000) {
        testVoltage = 220.0 + (random(0, 1000) / 100.0); 
        testCurrent = random(100, 1500) / 10.0;         
        testPower = (testVoltage * testCurrent) / 1000.0;
        
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(20, 0); display.println("POWER SENTINEL");
        display.drawFastHLine(0, 12, 128, SH110X_WHITE);

        display.setCursor(0, 22); display.print("Line V:  "); display.print(testVoltage, 1); display.println(" V");
        display.setCursor(0, 37); display.print("Load I:  "); display.print(testCurrent, 2); display.println(" mA");
        display.setCursor(0, 52); display.print("Usage P: "); display.print(testPower, 2); display.println(" W");
        display.display();
        
        lastOledUpdate = now;
      }
      break;

    // ==========================================
    // STATE 3: THEFT ALERT
    // ==========================================
    case 3:
      // LEDs & Buzzer: Blinking Red every 250ms
      if (now - lastBlink > 250) {
        blinkIsOn = !blinkIsOn;
        lastBlink = now;
        
        if (blinkIsOn) {
          for(int i=0; i<NUM_LEDS; i++) ring.setPixelColor(i, ring.Color(255, 0, 0));
          tone(BUZZ_PIN, 1200, 100); // Aggressive short beep synchronized with the blink
        } else {
          for(int i=0; i<NUM_LEDS; i++) ring.setPixelColor(i, 0); // Turn LEDs off
        }
        ring.show();
      }

      // OLED: Flashing Alert Text
      if (now - lastOledUpdate > 500) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(20, 0); display.println("POWER SENTINEL");
        display.drawFastHLine(0, 12, 128, SH110X_WHITE);
        
        display.setTextSize(2);
        display.setCursor(30, 25); display.println("THEFT");
        display.setCursor(30, 45); display.println("ALERT");
        display.display();
        lastOledUpdate = now;
      }
      break;
  }
}