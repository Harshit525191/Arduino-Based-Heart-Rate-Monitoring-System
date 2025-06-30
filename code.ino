#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define pin connections
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_MOSI  23      // pins for oled
#define OLED_CLK   18
#define OLED_DC    16
#define OLED_CS    5
#define OLED_RESET 17

#define GSR_PIN 34 
#define Led_pin 15      // LED pin for GSR
#define PULSE_PIN 32    // Pulse sensor pin
#define LED_PIN 2       // Onboard LED for pulse blink

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         &SPI, OLED_DC, OLED_RESET, OLED_CS);

// BPM variables
int bpm = 0;
int threshold = 1000;  // Adjust based on your pulse waveform
bool pulseDetected = false;
unsigned long lastBeatTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(Led_pin, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println("OLED failed");
    while (1);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.display();

  Serial.println("Starting sensors...");
}

void loop() {
  // --- GSR Reading ---
  int gsrValue = analogRead(GSR_PIN);
  Serial.print("GSR: ");
  Serial.println(gsrValue);

  // LED logic for GSR (non-blocking)
  static unsigned long lastBlinkTime = 0;
  unsigned long now = millis();

  if (gsrValue>= 90 &&gsrValue< 200) {
    if (now - lastBlinkTime> 500) {
      digitalWrite(Led_pin, !digitalRead(Led_pin));
      lastBlinkTime = now;
    }
  } else if (gsrValue>= 200) {
    digitalWrite(Led_pin, HIGH);
  } else {
    digitalWrite(Led_pin, LOW);
  }

  // --- Pulse Reading & BPM Calculation ---
  int pulseValue = analogRead(PULSE_PIN);
  Serial.print("Pulse raw: ");
  Serial.println(pulseValue);

  // Blink onboard LED when pulse peak is detected
  if (pulseValue> threshold && !pulseDetected) {
    pulseDetected = true;
    unsigned long currentTime = millis();
    int beatInterval = currentTime - lastBeatTime;

    if (beatInterval> 300 &&beatInterval< 2000) {
      bpm = 60000 / beatInterval;
      lastBeatTime = currentTime;
    }

    digitalWrite(LED_PIN, HIGH);  // Flash onboard LED
  }

  if (pulseValue< threshold) {
    pulseDetected = false;
    digitalWrite(LED_PIN, LOW);
  }
  Serial.println(bpm);

  // --- Display Data ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("GSR: ");
  display.println(gsrValue);
  display.setCursor(0, 30);
  display.print("BPM: ");
  display.println(bpm);
  display.display();

  delay(500);  // Keep it fast (~100Hz sampling)
}


