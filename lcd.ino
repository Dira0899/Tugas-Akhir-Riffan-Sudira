#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Ganti alamat I2C jika perlu (umumnya 0x27 atau 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Wire.begin();           // Inisialisasi I2C
  lcd.begin();            // Mulai LCD
  lcd.backlight();        // Nyalakan lampu latar LCD

  lcd.setCursor(0, 0);
  lcd.print("Riffan Sudira");
  lcd.setCursor(0, 1);
  lcd.print("2010442007");
}

void loop() {
  // Tidak perlu isi apa-apa untuk test sederhana
}
