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

#define MQ3_PIN 34

LiquidCrystal_I2C lcd(0x27, 16, 2);

float vinMQ3 = 0.0, alkohol = 0.0;

void bacaMQ3() {
  int mq3ADC = analogRead(MQ3_PIN);
  float voutMQ3 = mq3ADC * 3.3 / 4095.0;

  float R1 = 1000.0;
  float R2 = 2000.0;
  vinMQ3 = voutMQ3 * (R1 + R2) / R2;

  alkohol = 20.036 * vinMQ3 + 0.08;

  Serial.printf("MQ3 ADC: %d | Vin: %.3fV | Alkohol: %.2f%%\n", mq3ADC, vinMQ3, alkohol);

  lcd.setCursor(0, 0);
  lcd.print("Sensor MQ-3");
  lcd.setCursor(0, 1);
  lcd.printf("A: %.2f%%     ", alkohol);

  Blynk.virtualWrite(V3, alkohol); // V3 = Alkohol
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
  timer.setInterval(5000L, bacaMQ3);
}

void loop() {
  Blynk.run();
  timer.run();
}
