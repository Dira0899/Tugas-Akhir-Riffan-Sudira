// Tugas Akhir Riffan Sudira

#define BLYNK_TEMPLATE_ID "TMPL6pJ1afFvu"
#define BLYNK_TEMPLATE_NAME "Tugas akhir Riffan Sudira"
#define BLYNK_AUTH_TOKEN "nXYi56Oku-9Fpb18RSruHnFn9MNtQy8H"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <HTTPClient.h>

// WiFi
const char* ssid = "Tugas Akhir";
const char* password = "Riffan Sudira";

// Google Sheets URL
const char* scriptURL = "https://script.google.com/macros/s/AKfycbyJpkfQMR1J7GFstJteUms3BgXgV5T6sPf8JlH7lG83WOVWwHT3JhLLtGaMo6hBDmEB/exec";

BlynkTimer timer;

// Sensor
#define ONE_WIRE_BUS 4
#define MQ3_PIN 34
#define PH_PIN 36

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Output
#define FAN_PIN     18
#define HEATER_PIN  19
#define FAN_CH      0
#define HEATER_CH   1
#define PWM_FREQ    5000
#define PWM_RES     8

// Setpoint
const float setpoint = 29.0;

// Membership Function
#define MF_SIZE 5
float tempMF[MF_SIZE];
float errorMF[MF_SIZE];

// Nilai terakhir untuk Google Sheets
float lastTemp = 0, lastHeater = 0, lastFan = 0;
float alkohol = 0.0, phValue = 0.0; 
float vinMQ3 = 0.0;


// Membership Functions
float trapmf(float x, float a, float b, float c, float d) {
  float val1 = (x - a)/(b - a);
  float val2 = (d - x)/(d - c);
  float min1 = val1 < 1.0f ? val1 : 1.0f;
  float min2 = min1 < val2 ? min1 : val2;
  return min2 > 0.0f ? min2 : 0.0f;
}

float trimf(float x, float a, float b, float c) {
  if (x <= a || x >= c) return 0.0f;
  else if (x == b) return 1.0f;
  else if (x < b) return (x - a) / (b - a);
  else return (c - x) / (c - b);
}

void fuzzify(float temp, float error) {
  tempMF[0] = trapmf(temp, 20, 20, 23, 26);
  tempMF[1] = trimf(temp, 23, 26, 29);
  tempMF[2] = trimf(temp, 26, 29, 32);
  tempMF[3] = trimf(temp, 29, 32, 35);
  tempMF[4] = trapmf(temp, 32, 35, 40, 40);

  error = constrain(error, -5, 5);
  errorMF[0] = trapmf(error, -5, -5, -4, -2);
  errorMF[1] = trimf(error, -4, -2, 0);
  errorMF[2] = trimf(error, -2, 0, 2);
  errorMF[3] = trimf(error, 0, 2, 4);
  errorMF[4] = trapmf(error, 2, 4, 5, 5);
}

void readAlkoholAndPH() {
  int mq3ADC = analogRead(MQ3_PIN);
  int phADC = analogRead(PH_PIN);

  float voutMQ3 = mq3ADC * 3.3 / 4095.0;
  float voutPH = phADC * 3.3 / 4095.0;

  float R1_MQ3 = 1000.0;
  float R2_MQ3 = 2000.0;
  vinMQ3 = voutMQ3 * (R1_MQ3 + R2_MQ3) / R2_MQ3;

  float R1_PH = 10000.0;
  float R2_PH = 20000.0;
  float vinPH = (voutPH * (R1_PH + R2_PH) / R2_PH)-1.7684;

  alkohol = 20.036 * vinMQ3 + 0.08;
  phValue = -5.2799 * vinPH + 20.165;

  Serial.printf("MQ3 ADC: %d | Vout: %.3fV | Vin: %.3fV | Alkohol: %.2f%%\n", mq3ADC, voutMQ3, vinMQ3, alkohol);
  Serial.printf("PH  ADC: %d | Vout: %.3fV | Vin: %.3fV | pH: %.2f\n", phADC, voutPH, vinPH, phValue);
}

void loopControl() {
  sensors.requestTemperatures();
  readAlkoholAndPH();

  float temp = sensors.getTempCByIndex(0);
  float error = constrain(setpoint - temp, -5, 5);
  fuzzify(temp, error);

  float sumW = 0, heaterOut = 0, fanOut = 0;

  // 14 fuzzy rules 
  { float w = fmin(tempMF[0], errorMF[4]); heaterOut += w * 100; fanOut += w * 20; sumW += w; } //RK1
  { float w = fmin(tempMF[0], errorMF[3]); heaterOut += w * 80;  fanOut += w * 20; sumW += w; } //RK2
  { float w = fmin(tempMF[0], errorMF[2]); heaterOut += w * 60;  fanOut += w * 20; sumW += w; } //RK3
  { float w = fmin(tempMF[1], errorMF[4]); heaterOut += w * 80;  fanOut += w * 20; sumW += w; } //RK4
  { float w = fmin(tempMF[1], errorMF[3]); heaterOut += w * 60;  fanOut += w * 20; sumW += w; } //RK5
  { float w = fmin(tempMF[1], errorMF[2]); heaterOut += w * 40;  fanOut += w * 20; sumW += w; } //RK6
  { float w = fmin(tempMF[2], errorMF[3]); heaterOut += w * 40;  fanOut += w * 20; sumW += w; } //RK7
  { float w = fmin(tempMF[2], errorMF[2]); heaterOut += w * 20;  fanOut += w * 20; sumW += w; } //RK8
  { float w = fmin(tempMF[2], errorMF[1]); heaterOut += w * 20;  fanOut += w * 40; sumW += w; } //RK9
  { float w = fmin(tempMF[3], errorMF[2]); heaterOut += w * 20;  fanOut += w * 40; sumW += w; } //RK10
  { float w = fmin(tempMF[3], errorMF[1]); heaterOut += w * 20;  fanOut += w * 60; sumW += w; } //RK11
  { float w = fmin(tempMF[3], errorMF[0]); heaterOut += w * 20;  fanOut += w * 80; sumW += w; } //RK12
  { float w = fmin(tempMF[4], errorMF[1]); heaterOut += w * 20;  fanOut += w * 80; sumW += w; } //RK13
  { float w = fmin(tempMF[4], errorMF[0]); heaterOut += w * 20;  fanOut += w * 100; sumW += w; } //RK14

  if (sumW != 0) {
    heaterOut /= sumW;
    fanOut /= sumW;
  }

  uint8_t heaterPWM = map(heaterOut, 0, 100, 0, 255);
  uint8_t fanPWM = map(fanOut, 0, 100, 0, 255);
  ledcWrite(HEATER_CH, heaterPWM);
  ledcWrite(FAN_CH, fanPWM);

  lcd.setCursor(0, 0);
  lcd.printf("T:%.0f H:%02.0f F:%02.0f", temp, heaterOut, fanOut);
  lcd.setCursor(0, 1);
  lcd.printf("pH:%.2f A:%.2f%%", phValue, alkohol);

  // Simpan nilai terakhir untuk Google Sheets
  lastTemp = temp;
  lastHeater = heaterOut;
  lastFan = fanOut;

  // Kirim ke Blynk
  Blynk.virtualWrite(V0, lastTemp);
  Blynk.virtualWrite(V1, lastHeater);
  Blynk.virtualWrite(V2, lastFan);
  Blynk.virtualWrite(V3, alkohol);
  Blynk.virtualWrite(V4, phValue);
}

void sendToGoogleSheets() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = scriptURL;
    url += "?temp=" + String(lastTemp, 2);
    url += "&heater=" + String(lastHeater, 2);
    url += "&fan=" + String(lastFan, 2);
    url += "&ph=" + String(phValue, 2);
    url += "&alkohol=" + String(alkohol, 2);
    http.begin(url);
    http.GET();
    http.end();
  }
}

void setup() {
  Serial.begin(115200);

  // === Inisialisasi LCD ====
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Menyambung ke");
  lcd.setCursor(0, 1);
  lcd.print("WiFi...");

  WiFi.begin(ssid, password);
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.setCursor(9, 1);
    lcd.print(String(".").substring(0, dotCount % 4));
    dotCount++;
  }

  Serial.println("\nWiFi Connected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Terhubung!");
  delay(1000);
  lcd.clear();

  // === Inisialisasi Blynk dan komponen lainnya ===
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  sensors.begin();

  ledcSetup(FAN_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(FAN_PIN, FAN_CH);
  ledcSetup(HEATER_CH, PWM_FREQ, PWM_RES);
  ledcAttachPin(HEATER_PIN, HEATER_CH);

  timer.setInterval(5000L, loopControl);           
  timer.setInterval(600000L, sendToGoogleSheets);  
}


void loop() {
  Blynk.run();
  timer.run();
}
