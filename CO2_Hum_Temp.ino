#include <SoftwareSerial.h>
#include <Wire.h>
#include <DHT.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

#define MH_PIN_RX 8
#define MH_PIN_TX 9
#define DHT_PIN A0
#define PIEZO_PIN A3
#define DHT_TYPE DHT22
#define I2C_ADDRESS 0x3C
#define PERIOD_SEC 10
#define FONT ZevvPeep8x16

DHT dht(DHT_PIN, DHT_TYPE);
SSD1306AsciiAvrI2c display;
SoftwareSerial mhSerial(MH_PIN_RX, MH_PIN_TX);

byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte response[9];

long uptime = 0;

void setup() {
  delay(3000);
  Serial.begin(9600);
  mhSerial.begin(9600);
  Wire.begin();
  Wire.setClock(400000L);
  display.begin(&Adafruit128x32, I2C_ADDRESS);
  display.setFont(FONT);
  dht.begin();
}

void loop()
{
  //MH-Z19B
  mhSerial.write(cmd, 9);  
  memset(response, 0, 9);
  mhSerial.readBytes(response, 9);
  byte crc = 0;
  for (int i = 0; i < 8; i++) crc += response[i];
  crc = 255 - crc;

  //DHT
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  //MAIN
  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / " + String(response[8]));
    display.setCursor(0, 0);
    display.println("MH-Z19 CRC error");
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256 * responseHigh) + responseLow;

    Serial.println("Uptime: " + String(uptime / 60) + "min., CO2: " + String(ppm) + "ppm, Hum: " + String(hum) + "%, Temp: " + String(temp) + "C");
    if (ppm <= 400 || ppm > 4900 || isnan(hum) || isnan(temp)) {
      display.setCursor(0, 0);
      display.println("Sensor error!");
    } else {
      display.setCursor(0, 0);
      display.print("H:" + String(hum,1) + "%  T:" + String(temp,1) + "C");
      display.setCursor(0, 2);
      display.print("CO2:");
      display.setCursor(31, 2);
      if (ppm < 600) {
        display.println(String(ppm) + " NICE");
      }
      else if (ppm < 1000) {
        display.println(String(ppm) + " GOOD");
      }
      else if (ppm < 1600) {
        display.println(String(ppm) + " is BAD!");
        tone(PIEZO_PIN, 200, 10);
      }
      else if (ppm < 2500) {        
        display.println(String(ppm) + " CRITIC!");
        tone(PIEZO_PIN, 500, 50);
      }
      else {
        display.println(String(ppm) + " !ALERT!");
        tone(PIEZO_PIN, 1000, 2000);
      }
    }
  }
  delay(PERIOD_SEC*1000);
  uptime += PERIOD_SEC;
}
