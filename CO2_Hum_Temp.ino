#include <SoftwareSerial.h>
#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>
#include <DHT.h>

#define DHTPIN A0
#define DHTTYPE DHT22
#define I2C_ADDRESS 0x3C
#define PIEZOPIN A3
#define FONT ZevvPeep8x16

DHT dht(DHTPIN, DHTTYPE);
SSD1306AsciiWire display;
SoftwareSerial mySerial(8, 9);

byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
unsigned char response[9];

long uptime = 0;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);

  Wire.begin();
  display.begin(&Adafruit128x32, I2C_ADDRESS);
  display.set400kHz();
  display.setFont(FONT);
  //display.println("setup::init()");

  dht.begin();
}

void loop()
{
  //MH-Z19B
  mySerial.write(cmd, 9);
  memset(response, 0, 9);
  mySerial.readBytes(response, 9);
  int i;
  byte crc = 0;
  for (i = 1; i < 8; i++) crc += response[i];
  crc = 255 - crc;
  crc++;

  //DHT
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();

  //MAIN
  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / " + String(response[8]));
    display.println("MH-Z19 CRC error");
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256 * responseHigh) + responseLow;

    Serial.println("Uptime: " + String(uptime / 60) + "min., CO2: " + String(ppm) + "ppm, Hum: " + String(hum) + "%, Temp: " + String(temp) + "C");

    if (ppm <= 400 || ppm > 4900 || isnan(hum) || isnan(temp)) {
      display.println("Sensor error!");
    } else {
      display.println("H:" + String(hum,1) + "%  T:" + String(temp,1) + "C");
      if (ppm < 600) {
        display.println("CO2:" + String(ppm) + " NICE");
      }
      else if (ppm < 1000) {
        display.println("CO2:" + String(ppm) + " GOOD");
      }
      else if (ppm < 1600) {
        display.println("CO2:" + String(ppm) + " is BAD!");
        //tone(PIEZOPIN, 1000, 500);
      }
      else if (ppm < 2500) {
        display.println("CO2:" + String(ppm) + " CRITIC!");
      }
      else {
        display.println("CO2:" + String(ppm) + " !ALERT!");
      }
    }
  }
  delay(10000);
  uptime += 10;
}
