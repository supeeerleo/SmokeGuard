#define BLYNK_TEMPLATE_ID "TMPL6t-Y4BWBE"
#define BLYNK_TEMPLATE_NAME "Smoke sensor"
#define BLYNK_AUTH_TOKEN "yourtoken"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <LiquidCrystal.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "yourwifi";
char pass[] ="yourpassword";

// LCD without potentiometer - fixed contrast
const int rs = 26, en = 27, d4 = 14, d5 = 12, d6 = 13, d7 = 15;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// MQ-135
const int smokeA0 = 34;
const float R0 = 10000.0;
const float RL = 20000.0;
const float Vcc = 5.0;

// Buzzer
#define BUZZER_PIN 25
const int freq = 2000;  // 2kHz
const int channel = 0;
const int resolution = 8;

int data;
BlynkTimer timer;

// Function to compute PPM from MQ-135 analog output
float getSmokePPM() {
  int rawValue = analogRead(smokeA0);
  float voltage = (rawValue / 4095.0) * 3.3;
  float Rs = ((Vcc * RL) / voltage) - RL;
  float ratio = Rs / R0;
  float ppm = 100.0 * pow(ratio, -1.3);
  return ppm;
}

// Function to sound buzzer
void buzz(bool on) {
  if (on) {
    ledcWrite(channel, 127); // 50% duty
  } else {
    ledcWrite(channel, 0);
  }
}

// Send sensor data + display + alert
void sendSensor() {
  float ppm = getSmokePPM();
  data = (int)ppm;

  lcd.clear();
  lcd.setCursor(0, 0);

  // Determine gas or smoke
  if (ppm > 11) {
    lcd.print("Gas Detected!");
    Blynk.logEvent("smoke_detected", "Gas Detected!");
    buzz(true);
  } else if (ppm >= 8 && ppm <= 10) {
    lcd.print("High Smoke");
    Blynk.logEvent("smoke_detected", "High Smoke!");
    buzz(true);
  } else if (ppm >= 5 && ppm < 8) {
    lcd.print("Moderate Smoke");
    Blynk.logEvent("smoke_detected", "Moderate Smoke!");
    buzz(true);
  } else if (ppm >= 3 && ppm < 5) {
    lcd.print("Low Smoke");
    Blynk.logEvent("smoke_detected", "Low Smoke!");
    buzz(true);
  } else {
    lcd.print("Air is Clean");
    buzz(false);
  }

  lcd.setCursor(0, 1);
  lcd.print("PPM: ");
  lcd.print((int)ppm);

  Blynk.virtualWrite(V0, ppm);
}

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  lcd.begin(16, 2);

  pinMode(smokeA0, INPUT);

  // Buzzer PWM setup
  ledcSetup(channel, freq, resolution);
  ledcAttachPin(BUZZER_PIN, channel);
  buzz(false); // ensure buzzer is off

  timer.setInterval(3000L, sendSensor);
}

void loop() {
  Blynk.run();
  timer.run();
}
