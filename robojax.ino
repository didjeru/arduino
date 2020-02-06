#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
double count=0;

/*
 * PIN connection:
 * pin connection see: https://www.arduino.cc/en/Reference/Wire
 * for UNO: SDA to A4, SCL to A5
 * for Mega2560: SDA to 20, SCL to 21
 * for Leonardo: SDA to 2, SCL to 3
 * for Due: SDA to 20, SCL to 21
 * VCC to 5V
 * GND to GND :-)
 */



#define LOGO16_GLCD_HEIGHT 16 // do not change this. Error in video
#define LOGO16_GLCD_WIDTH  16 // do not change this. Error in video

    // look at line 27 to 30 of Adafruit_SSD1306.h inside the library to select the dimensions
#if (SSD1306_LCDHEIGHT != 32) // 
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {                
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  delay(2000);
   // Clear the buffer.
  display.clearDisplay(); 
}


void loop() {
  String vString =  String(count, 4);// using a float and the 
  display.clearDisplay();
  robojaxText("Voltage:          ", 4, 3, 1, false);
  robojaxText(vString, 72, 3, 1, false);
  robojaxText("V", 110, 3, 1, false);
  robojaxText("Temprature: 32C", 4, 11, 1, false);
  robojaxText("Capacity:   92.86L", 4, 21, 1, false);
  //display.drawLine(1, 37, 100, 37, WHITE);
 display.drawRect(1, 1, 126,31, WHITE);
  //display.drawCircle(63,31, 31, WHITE);
   //display.startscrollright(0x00, 0x0F);
  display.display();
  count +=0.173;
   delay(2000); 
}


/*
 * robojaxText(String text, int x, int y,int size, boolean d)
 * text is the text string to be printed
 * x is the integer x position of text
 * y is the integer y position of text
 * z is the text size, 1, 2, 3 etc
 * d is either "true" or "false". Not sure, use true
 */
void robojaxText(String text, int x, int y,int size, boolean d) {

  display.setTextSize(size);
  display.setTextColor(WHITE);
  display.setCursor(x,y);
  display.println(text);
  if(d){
    display.display();
  }
  
  //delay(100);
}
