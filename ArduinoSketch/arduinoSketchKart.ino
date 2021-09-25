//  **********************
//  *** Arduino Sketch ***
//  **********************

#include <SoftwareSerial.h>
#include <Wire.h>
#include "ClosedCube_HDC1080.h"

// Hudimity/Temp region
uint32_t timerHT;
int periodHT = 2000;
ClosedCube_HDC1080 hdc1080;

SoftwareSerial espSerial(5, 6);
String str;

void setup(){
Serial.begin(115200);
espSerial.begin(115200);

startHT(); // Запуск HDC1080

delay(2000);
}
void loop()
{
  // Hudimity/Temp region
  uint32_t timeLeftHT = millis() - timerHT;
  if (timeLeftHT >= periodHT) {
    timerHT += periodHT * (timeLeftHT / periodHT);
    str = String("HT_P:")+String(hdc1080.readHumidity())+String(";")+String(hdc1080.readTemperature());
    espSerial.println(str);
    Serial.println(str);
  }
// endregion

}

void startHT() {
  Serial.println("Start HDC1080");
    hdc1080.begin(0x40);
  Serial.print("Manufacturer ID=0x");
  Serial.println(hdc1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
  Serial.print("Device ID=0x");
  Serial.println(hdc1080.readDeviceId(), HEX); // 0x1050 ID of the device
    Serial.print("Device Serial Number=");
  HDC1080_SerialNumber sernum = hdc1080.readSerialNumber();
  char format[12];
  sprintf(format, "%02X-%04X-%04X", sernum.serialFirst, sernum.serialMid, sernum.serialLast);
  Serial.println(format);
  Serial.println("********************************");
  }
