#include <SoftwareSerial.h>
#include <DHT.h>
#include <Adafruit_BME280.h>
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

Adafruit_BME280 bme;
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
  display.begin(&Adafruit128x32, I2C_ADDRESS);
  display.setFont(FONT);
  dht.begin();
  bme.begin();
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

  //Hum Temp
  float hum1 = dht.readHumidity();
  float temp1 = dht.readTemperature();
  float hum = bme.readHumidity();
  float temp = bme.readTemperature();

  //MAIN
  display.setCursor(0, 0);
  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / " + String(response[8]));
    display.clear();
    display.println("MH-Z19 calibrate\nCRC: " + String(crc) + "/" + String(response[8]));
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256 * responseHigh) + responseLow;
    float alt = (bme.readAltitude(1009.8));
    float hpa = ((bme.readPressure() / 100) * 0.750062);

    Serial.println("Uptime: " + String(uptime / 60) + " min." + " CO2: " + String(ppm) + " ppm," 
                   + " Humidity: " + String(hum) + " %" + " Temperature: " + String(temp) + " C"
                   + " HumidityDHT: " + String(hum1) + " %" + " TemperatureDHT: " + String(temp1) + " C" 
                   + " Pressure: " + String(hpa) + " Altitude: " + String(alt) + " m");
    if (ppm <= 350 || ppm > 4950 || isnan(hum) || isnan(temp)) {
      display.println("Sensor error!");
    } else {
      display.print("H:" + String(hum, 1) + "%  T:" + String(temp, 1) + "C");
      display.setCursor(0, 2);
      display.print("CO2:");
      display.setCursor(31, 2);
      if (ppm < 600) {
        display.println(String(ppm) + "  " + String(hpa, 0) + "/" + String(alt, 0));
      }
      else if (ppm < 1000) {
        display.println(String(ppm) + "  " + String(hpa, 0) + "/" + String(alt, 0));
      }
      else if (ppm < 1600) {
        display.println(String(ppm) + " " + String(hpa, 0) + "/" + String(alt, 0));
        tone(PIEZO_PIN, 200, 1);
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
  delay(PERIOD_SEC * 1000);
  uptime += PERIOD_SEC;
}
