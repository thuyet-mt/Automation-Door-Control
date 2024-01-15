#include "declare_main.h"
#include "declare_finger.h"
#include "declare_keypad.h"
#include "declare_lcd.h"
#include "declare_menu.h"
#include "declare_mqtt.h"
#include "declare_rfid.h"


uint8_t eventMenu;
int flagPassword = 0;
int flagFingerprint = 0;
int flagRFID = 0;

TaskHandle_t taskCheckRFIDHandle;
TaskHandle_t taskCheckKeyPadHandle;
TaskHandle_t taskCheckFingerprintIDHandle;
TaskHandle_t taskAddRFIDHandle;
TaskHandle_t taskAddFingerprintHandle;

extern LiquidCrystal_I2C lcd;
extern MFRC522 rfid;
extern Adafruit_Fingerprint finger;


void setup()
{
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  setUpFingerprint();
  SPI.begin();     // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  setupConnection();

  while (flagPassword == 0 || flagFingerprint == 0 || flagRFID == 0)
  {
    displayMenu();
    eventMenu = readNumberKeypad();
    switch (eventMenu)
    {
    case 1:
      registerPassword();
      flagPassword = 1;
      Serial.print("flagPassword:");
      Serial.println(flagPassword);
      break;
    case 2:
      registerFingerprint();
      flagFingerprint = 1;
      Serial.print("flagFingerprint:");
      Serial.println(flagFingerprint);
      break;
    case 3:
      registerRFID();
      flagRFID = 1;
      Serial.print("flagRFID:");
      Serial.println(flagRFID);
      break;
    }
  }
  lcd.clear();
  lcd.setCursor(1, 1);
  lcd.print("Register done!!!");
  xTaskCreate(taskCheckRFID, "Task check RFID", 4096*2, NULL, 3, &taskCheckRFIDHandle);
  xTaskCreate(taskCheckKeyPad, "Task check Keypad", 4096*2, NULL, 3, &taskCheckKeyPadHandle);
  xTaskCreate(taskCheckFingerprint, "Task check Fingerprint", 4096*2, NULL, 3, &taskCheckFingerprintIDHandle);
  xTaskCreate(taskAddRFID, "Task Add RFID", 4096, NULL, tskIDLE_PRIORITY, &taskAddRFIDHandle);
  xTaskCreate(taskAddFingerprint, "Task Add Fingerprint", 4096, NULL, tskIDLE_PRIORITY, &taskAddFingerprintHandle);
}

void loop()
{
}