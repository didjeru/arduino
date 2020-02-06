#include "SoftwareSerial.h"
#include "Wire.h"

//I2C OLED:
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

//HUM and TEMP:
#include "DHT.h"
#define DHTPIN A0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//CO2 sensor:
SoftwareSerial mySerial(8, 9); // RX,TX
byte cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
unsigned char response[9];

void setup() {
  //Serial
  Serial.begin(9600);

  //DHT
  mySerial.begin(9600);

  //OLED
  Wire.begin();
  oled.begin(&Adafruit128x32, I2C_ADDRESS);
  oled.set400kHz();
  oled.setFont(ZevvPeep8x16);
  oled.clear();
  oled.println("setup::init()");

  //DHT
  dht.begin();
}

//piezo
int piezoPin = A3;
long t = 0;

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
  if (isnan(hum) || isnan(temp)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    oled.println("Sensor DHT error");
    return;
  }

  //OLED
  oled.clear();
  if ( !(response[0] == 0xFF && response[1] == 0x86 && response[8] == crc) ) {
    Serial.println("CRC error: " + String(crc) + " / " + String(response[8]));
    oled.println("Sensor CRC error");
  } else {
    unsigned int responseHigh = (unsigned int) response[2];
    unsigned int responseLow = (unsigned int) response[3];
    unsigned int ppm = (256 * responseHigh) + responseLow;
    Serial.print(String(t)); Serial.print(","); Serial.print(ppm); Serial.println(";");
    if (ppm <= 400 || ppm > 4900) {
      oled.println("CO2: no data");
    } else {
      oled.println("H:" + String(hum) + "% T:" + String(round(temp)) + "C");
      if (ppm < 600) {
        oled.println("CO2:" + String(ppm) + " NICE");
      }
      else if (ppm < 1000) {
        oled.println("CO2:" + String(ppm) + " GOOD");
      }
      else if (ppm < 1600) {
        oled.println("CO2:" + String(ppm) + " BAD CO2");
        //tone(piezoPin, 1000, 500);
      }
      else if (ppm < 2500) {
        oled.println("CO2:" + String(ppm) + " CRITIC!");
      }
      else {
        oled.println("CO2" + String(ppm) + " !ALERT!");
      }
    }
  }
  delay(100000);
  t += 10;
}
