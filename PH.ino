#define BLYNK_TEMPLATE_ID "TMPL6pJ1afFvu"
#define BLYNK_TEMPLATE_NAME "Tugas akhir Riffan Sudira"
#define BLYNK_AUTH_TOKEN "nXYi56Oku-9Fpb18RSruHnFn9MNtQy8H"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

BlynkTimer timer;

const char* ssid = "Tugas Akhir";
const char* password = "Riffan Sudira";

#define PH_PIN 36

LiquidCrystal_I2C lcd(0x27, 16, 2);

float phValue = 0.0;

void bacaPH() {
  int phADC = analogRead(PH_PIN);
  float voutPH = phADC * 3.3 / 4095.0;

  float R1 = 10000.0;
  float R2 = 20000.0;
  float vinPH = (voutPH * (R1 + R2) / R2) - 1.7684;

  phValue = -5.2799 * vinPH + 20.165;

  Serial.printf("PH ADC: %d | Vin: %.3fV | pH: %.2f\n", phADC, vinPH, phValue);

  lcd.setCursor(0, 0);
  lcd.print("Sensor pH");
  lcd.setCursor(0, 1);
  lcd.printf("pH: %.2f     ", phValue);

  Blynk.virtualWrite(V4, phValue); // V4 = pH
}

void setup() {
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Menghubungkan");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Terhubung");
  delay(1000);
  lcd.clear();

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  timer.setInterval(5000L, bacaPH);
}

void loop() {
  Blynk.run();
  timer.run();
}
