//
// Touchpad calibration program for Adafruit Touch Shield V2
//
//    - determines touchpad constants TS_MINX, TS_MINY, TS_MAXX, & TS_MAXY on-the-fly
//    - use something small but not too sharp (e.g. toothpick) to poke the touchscreen
//
// V1.1 by MJCulross (KD5RXT) - 20240723-2330
//
// Uses the Adafruit Touch Shield V2 library
//    for its display output & touchpad input
//
//

// uncomment the following line to enable using the older displays using the STMPE610 touch controller
// - OR - comment out the following line to enable using the newer displays using the TSC2007 touch controller
//#define STMPE610_TOUCH_CONTROLLER


// The following pins are used in this project:
//
// PIN  D2  = (not used)
// PIN  D3  = TFT backlight control (optional - solder jumper to activate)
// PIN  D4  = microSD chip select
// PIN  D5  = (not used)
// PIN  D6  = (not used)
// PIN  D7  = (not used)
// PIN  D8  = Resistive TouchScreen chip select
// PIN  D9  = TFT data/command select
// PIN D10  = TFT chip select
// PIN D11  = SPI data input pin - used for TFT, microSD, & Resistive TouchScreen data/command
// PIN D12  = SPI data output pin - used for TFT, microSD, & Resistive TouchScreen data/command
// PIN D13  = SPI serial clock pin - used for TFT, microSD, & Resistive TouchScreen clock
// PIN  A0  = (not used)
// PIN  A1  = (not used)
// PIN  A2  = (not used)
// PIN  A3  = (not used)
// PIN  A4  = (not used)
// PIN  A5  = (not used)

#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>
#include <Wire.h>      // this is needed even tho we aren't using it
#include <Adafruit_ILI9341.h>

#ifdef STMPE610_TOUCH_CONTROLLER
#include <Adafruit_STMPE610.h>
#else
#include <Adafruit_TSC2007.h>
#endif

// This is rough calibration data for the raw touch data to the screen coordinates
#define TS_MINX 210
#define TS_MINY 200
#define TS_MAXX 3860
#define TS_MAXY 3760

#ifdef STMPE610_TOUCH_CONTROLLER
// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);
#else
Adafruit_TSC2007 ts;

#define TS_MIN_PRESSURE 200
#endif

// The display uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

const int speaker = 2;   // define the speaker pin

uint16_t sampleX[10];
uint16_t sampleY[10];
int indexX = 0;
int indexY = 0;

void setup()
{
   tft.begin();  //init TFT library
   ts.begin();   //init TouchScreen library

   // blank the whole display
   tft.fillRect(0, 0, 240, 320, ILI9341_WHITE);
}


void loop()
{
   uint16_t rawX, rawY, rawZ1, rawZ2;
   uint8_t rawZ;

   uint16_t avgX = 0;
   uint16_t avgY = 0;

   tft.setTextColor(ILI9341_BLACK);
   tft.setTextSize(1);

   for (int i = 0; i < 9; i++)
   {
      sampleX[i] = 200;
      sampleY[i] = 200;
   }

   tft.fillRect(0, 200, 240, 40, ILI9341_WHITE);

   tft.setCursor(20, 20);
   tft.println("Touch the top-left corner...");

#ifdef STMPE610_TOUCH_CONTROLLER
   // wait for touch
   while (! ts.touched()) {}

   // wait for touch to go away
   while (ts.touched())
   {
      ts.readData(&rawX, &rawY, &rawZ);
#else
   // wait for touch
   while (ts.read_touch(&rawX, &rawY, &rawZ1, &rawZ2) && (rawZ1 < TS_MIN_PRESSURE))
   {
   }

   while (ts.read_touch(&rawX, &rawY, &rawZ1, &rawZ2) && (rawZ1 > TS_MIN_PRESSURE))
   {
#endif
      sampleX[indexX] = rawX;
      if (++indexX > 9)
      {
         indexX = 0;
      }
      for (int i = 0; i < 9; i++)
      {
         avgX += sampleX[i];
      }
      avgX /= 10;

      sampleY[indexY] = rawY;
      if (++indexY > 9)
      {
         indexY = 0;
      }
      for (int i = 0; i < 9; i++)
      {
         avgY += sampleY[i];
      }
      avgY /= 10;
   }

   tft.fillRect(0, 40, 240, 50, ILI9341_WHITE);

   tft.setCursor(20, 40);
   tft.println("rawX: ");
   tft.setCursor(50, 40);
   tft.println(rawX);
   tft.setCursor(20, 50);
   tft.println("rawY: ");
   tft.setCursor(50, 50);
   tft.println(rawY);

   tft.setCursor(20, 70);
   tft.println("TS_MINX: ");
   tft.setCursor(70, 70);
   tft.println(avgX);
   tft.setCursor(20, 80);
   tft.println("TS_MINY: ");
   tft.setCursor(70, 80);
   tft.println(avgY);

#ifdef STMPE610_TOUCH_CONTROLLER
   // now, empty any buffered data
   while (! ts.bufferEmpty())
#else
   while (ts.read_touch(&rawX, &rawY, &rawZ1, &rawZ2) && (rawZ1 > TS_MIN_PRESSURE))
#endif
   {
      TS_Point p_discard = ts.getPoint();
   }

   tft.fillRect(0, 20, 240, 40, ILI9341_WHITE);

   tft.setCursor(20, 200);
   tft.println("Touch the bottom-right corner...");

#ifdef STMPE610_TOUCH_CONTROLLER
   // wait for touch
   while (! ts.touched()) {}

   // wait for touch to go away
   while (ts.touched())
   {
      ts.readData(&rawX, &rawY, &rawZ);
#else
   // wait for touch
   while (ts.read_touch(&rawX, &rawY, &rawZ1, &rawZ2) && (rawZ1 < TS_MIN_PRESSURE))
   {
   }

   while (ts.read_touch(&rawX, &rawY, &rawZ1, &rawZ2) && (rawZ1 > TS_MIN_PRESSURE))
   {
#endif
      sampleX[indexX] = rawX;
      if (++indexX > 9)
      {
         indexX = 0;
      }
      for (int i = 0; i < 9; i++)
      {
         avgX += sampleX[i];
      }
      avgX /= 10;

      sampleY[indexY] = rawY;
      if (++indexY > 9)
      {
         indexY = 0;
      }
      for (int i = 0; i < 9; i++)
      {
         avgY += sampleY[i];
      }
      avgY /= 10;
   }


   tft.fillRect(0, 120, 240, 50, ILI9341_WHITE);

   tft.setCursor(20, 120);
   tft.println("rawX: ");
   tft.setCursor(50, 120);
   tft.println(rawX);
   tft.setCursor(20, 130);
   tft.println("rawY: ");
   tft.setCursor(50, 130);
   tft.println(rawY);

   tft.setCursor(20, 150);
   tft.println("TS_MAXX: ");
   tft.setCursor(70, 150);
   tft.println(avgX);
   tft.setCursor(20, 160);
   tft.println("TS_MAXY: ");
   tft.setCursor(70, 160);
   tft.println(avgY);

#ifdef STMPE610_TOUCH_CONTROLLER
   // now, empty any buffered data
   while (! ts.bufferEmpty())
#else
   while (ts.read_touch(&rawX, &rawY, &rawZ1, &rawZ2) && (rawZ1 > TS_MIN_PRESSURE))
#endif
   {
      TS_Point p_discard = ts.getPoint();
   }
}
