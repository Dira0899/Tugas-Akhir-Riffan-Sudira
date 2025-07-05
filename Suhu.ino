
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup(void)
{

  Serial.begin(9600);
  Serial.println("MONITORING SUHU");
  Wire.begin(21, 22); 
  sensors.begin();

  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("MONITORING SUHU");
  delay(2000);
  lcd.clear();
}
void loop(void)
{
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); 
  Serial.println("DONE");
  delay(1500);

  float tempC = sensors.getTempCByIndex(0);

  if (tempC != DEVICE_DISCONNECTED_C)
  {
    Serial.print("Suhu : ");
    Serial.println(tempC);


    lcd.setCursor(0, 0);
    lcd.print("Suhu: ");
    lcd.print(tempC);
    lcd.print((char)223); 
    lcd.print("C   ");
  }
  else
  {
    Serial.println("Error: Could not read temperature data");
    lcd.setCursor(0, 0);
    lcd.print("Temp Error");
  }

  delay(2000);
}
