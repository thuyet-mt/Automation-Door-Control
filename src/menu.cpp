#include "declare_menu.h"
#include "declare_lcd.h"

extern LiquidCrystal_I2C lcd;

void displayMenu(void)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome Smartkey !!");
    Serial.println("Welcome Smartkey !!");
    lcd.setCursor(0, 1);
    lcd.print("1. Register Pass");
    Serial.println("1. Register Pass");
    lcd.setCursor(0, 2);
    lcd.print("2. Register Finger");
    Serial.println("2. Register Finger");
    lcd.setCursor(0, 3);
    lcd.print("3. Register RFIDTag");
    Serial.println("3. Register RFIDTag");
}