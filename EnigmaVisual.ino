//
// Enigma Visual version 1.3 dated 03/10/2019 @2110
//    written by Mark J Culross, KD5RXT (kd5rxt@arrl.net)
//
// HARDWARE:
//
//    SparkFun RedBoard - Programmed with Arduino (DEV-13975)
//       https://www.sparkfun.com/products/13975
//
//    Adafruit 2.8" TFT Touch Shield for Arduino with Resistive Touch Screen (1651)
//       https://www.adafruit.com/product/1651
//
// Uses touchscreen input & color display to create a visual (graphical) version of the Enigma machine
//
// Uses the Adafruit Touch Shield V2 library for its display output & touchpad input
//
// The design of the encode/decode processing in this sketch is based upon the original encode/decode engine from
//    the EnigmaSerial.ino sketch (w/ cleanup, optimization, & enhancements, all w/ no apparent adverse effects on
//    equivalent/correct operation)
//
// The versions of the Enigma machine that are simulated in this sketch were verified to operate correctly using
//    the excellent/versatile HTML-code utility titled "Universal Enigma Simulator v2.0" by Daniel Palloks
//    ( http://people.physik.hu-berlin.de/~palloks/js/enigma/index_en.html )
//
// See text at the bottom of this file for sample Enigma messages (http://franklinheath.co.uk) for testing/verification
//
// COMPILER/LIBRARY NOTE:
//    If this version is built using older versions of the IDE and/or aome versions of the Adafruit TFT library,
//    the resulting compiled code could exceed the available program space.  When this version is built with IDE
//    version 1.8.7, the resulting code is efficient enough that it fits within the available program space.  If
//    you find that the available program space is exceeded when this program is built, then simply comment out
//    the line "#define SERIAL_MODE" (which appears immediately following this note), essentially removing the
//    calls for run-time serial input & output, which should thus avoid exceeding the available program space
//    until the IDE and/or Adafruit TFT library can be updated to newer versions.  Of course, doing so also
//    results in the loss of the otherwise available serial capabiltity for processing Enigma messages & the
//    ability to display the intricate details of the internal mechanical operations of the Enigma machine over
//    the serial port.
//

#define SERIAL_MODE

//
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
// PIN D11  = SPI data pin - used for TFT, microSD, & TouchScreen data/command
// PIN D12  = SPI data pin - used for TFT, microSD, & TouchScreen data/command
// PIN D13  = SPI serial clock pin - used for TFT, microSD, & TouchScreen clock
// PIN  A0  = (not used)
// PIN  A1  = (not used)
// PIN  A2  = (not used)
// PIN  A3  = (not used)
// PIN  A4  = (not used)
// PIN  A5  = (not used)

#include <SPI.h>
#include <Wire.h>            // this is needed even though we aren't using it directly
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>
#include <Adafruit_GFX.h>    // Core graphics library

// This is calibration data for the raw touch data to the screen coordinates
// (NOTE: run the TFTcal-Adafruit.ino sketch to determine the calibration values
//        for your specific touchscreen display)
const int TS_MINX = 150;
const int TS_MINY = 200;
const int TS_MAXX = 3830;
const int TS_MAXY = 3750;

// The STMPE610 uses hardware SPI on the shield, and pin #8 for ChipSelect
const int STMPE_CHIP_SELECT = 8;
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CHIP_SELECT);

// The display also uses hardware SPI, plus pin #9 as DataCommand & pin #10 as ChipSelect
const int TFT_CHIP_SELECT = 10;
const int TFT_DATA_COMMAND = 9;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CHIP_SELECT, TFT_DATA_COMMAND);

// define constant display strings
#define TITLE            F(" Visual Simulator ")
#define VERSION0         F(" v1.3 ")
#define VERSION1         F("written by: \n MarkCulross KD5RXT")
#define TAPSTART         F("Tap the Enigma touchscreen to begin...")
#define CREDIT1          F(" Many thanks to Daniel Palloks for his\n\n excellent/versatile HTML-code utility\n\n Universal Enigma Simulator v2.0 which\n\n was used to validate proper operation\n\n\n Also, thanks & credit to the authors\n\n of EnigmaSerial, whose encode/decode\n\n engine was used as a model to design\n\n my sketch's encode/decode processing")

// create a universal definition for the internal reference
//    (UNO includes standard definition for INTERNAL)
//    (MEGA does not have INTERNAL defined, but uses INTERNAL1V1 as same thing)
#ifndef INTERNAL
#define INTERNAL INTERNAL1V1
#endif

const int MY_ILI9341_SHADOW_GRAY      = 0x1923;
const int MY_ILI9341_DARK_GRAY        = 0x29C5;
const int MY_ILI9341_MEDIUM_GRAY      = 0x4A89;
const int MY_ILI9341_LIGHT_GRAY       = 0x8410;
const int MY_ILI9341_BRASS            = 0xBC20;
const int MY_ILI9341_PAPER            = 0xFDE7;
const int MY_ILI9341_ORANGE           = 0xFC00;

#define STR_EXPAND(tok) #tok
#define STR(tok) STR_EXPAND(tok)

typedef enum
{
   USE_ROTOR_NONE=0, USE_ROTOR_ETW, USE_ROTOR_1, USE_ROTOR_2, USE_ROTOR_3, USE_ROTOR_4, USE_ROTOR_5, USE_ROTOR_6, USE_ROTOR_7, USE_ROTOR_8,
   USE_ROTOR_UKWA, USE_ROTOR_UKWB, USE_ROTOR_UKWC, USE_ROTOR_B, USE_ROTOR_G, USE_ROTOR_UKWBD, USE_ROTOR_UKWCD,
   USE_ROTOR_ETWD, USE_ROTOR_D1, USE_ROTOR_D2, USE_ROTOR_D3, USE_ROTOR_UKWD,
   USE_ROTOR_ETWR, USE_ROTOR_R1, USE_ROTOR_R2, USE_ROTOR_R3, USE_ROTOR_UKWR,
   USE_ROTOR_ETWS, USE_ROTOR_S1, USE_ROTOR_S2, USE_ROTOR_S3, USE_ROTOR_UKWS,
}  ROTORS;

typedef enum
{
   MACHINE_TYPE_M3=0,        // Standard model Enigma M3
   MACHINE_TYPE_M4,          // Standard model Enigma M4
   MACHINE_TYPE_ENIGMA_D,    // Business model Enigma D
   MACHINE_TYPE_ROCKET_K,    // Enigma Rocket K Railway
   MACHINE_TYPE_SWISS_K      // Enigma Swiss K
}  MACHINE_TYPE;

typedef enum
{
   CONFIG_M3_1939 = 0,
   CONFIG_EXAMPLE_1930,
   CONFIG_ENIGMA_D,
   CONFIG_ROCKET_K_RAILWAY,
   CONFIG_SWISS_K,
   CONFIG_TURING_1940,
   CONFIG_BARBAROSA_1941,
   CONFIG_M4_1942,
   CONFIG_U264_1942,
   CONFIG_SCHARNHORST_1943
}  CONFIG;

int config_setup = CONFIG_M3_1939;

// Rotor definitions, first two letters are the letter at which they rotate the one to the left,
//    the rest are the output given an input letter
//
// Standard Enigma - rotor definitions from Crypto Museum
//                  ABCDEFGHIJKLMNOPQRSTUVWXYZ
#define     ETW   --ABCDEFGHIJKLMNOPQRSTUVWXYZ
#define  ROTOR1   R-EKMFLGDQVZNTOWYHXUSPAIBRCJ
#define  ROTOR2   F-AJDKSIRUXBLHWTMCQGZNPYFVOE
#define  ROTOR3   W-BDFHJLCPRTXVZNYEIWGAKMUSQO
#define  ROTOR4   K-ESOVPZJAYQUIRHXLNFTGKDCMWB
#define  ROTOR5   A-VZBRGITYUPSDNHLXAWMJQOFECK
#define  ROTOR6   ANJPGVOUMFYQBENHZRDKASXLICTW
#define  ROTOR7   ANNZJHGRCXMYSWBOUFAIVLPEKQDT
#define  ROTOR8   ANFKQHTLXOCBJSPDZRAMEWNIUYGV
#define    UKWA   --EJMZALYXVBWFCRQUONTSPIKHGD
#define    UKWB   --YRUHQSLDPXNGOKMIEBFZCWVJAT
#define    UKWC   --FVPJIAOYEDRZXWGCTKUQSBNMHL
#define  ROTORB   --LEYJVCNIXWPBQMDRTAKZGFUHOS
#define  ROTORG   --FSOKANUERHMBTIYCWLQPZXVGJD
#define   UKWBD   --ENKQAUYWJICOPBLMDXZVFTHRGS
#define   UKWCD   --RDOBJNTKVEHMLFCWZAXGYIPSUQ

// Business Enigma D - rotor definitions from Universal Enigma Simulator by Daniel Palloks
//                  ABCDEFGHIJKLMNOPQRSTUVWXYZ
#define    ETWD   --JWULCMNOHPQZYXIRADKEGVBTSF
#define  ROTORD1  Z-LPGSZMHAEOQKVXRFYBUTNICJDW
#define  ROTORD2  F-SLVGBTFXJQOHEWIRZYAMKPCNDU
#define  ROTORD3  O-CJGDPSHKTURAWZXFMYNQOBVLIE
#define    UKWD   --IMETCGFRAYSQBZXWLHKDVUPOJN

// German Rocket K Railway - rotor definitions from Universal Enigma Simulator by Daniel Palloks
//                  ABCDEFGHIJKLMNOPQRSTUVWXYZ
#define    ETWR   --JWULCMNOHPQZYXIRADKEGVBTSF
#define  ROTORR1  O-JGDQOXUSCAMIFRVTPNEWKBLZYH
#define  ROTORR2  F-NTZPSFBOKMWRCJDIVLAEYUXHGQ
#define  ROTORR3  Z-JVIUBHTCDYAKEQZPOSGXNRMWFL
#define    UKWR   --QYHOGNECVPUZTFDJAXWMKISRBL

// Swiss K - rotor definitions from Universal Enigma Simulator by Daniel Palloks
//                  ABCDEFGHIJKLMNOPQRSTUVWXYZ
#define    ETWS   --JWULCMNOHPQZYXIRADKEGVBTSF
#define  ROTORS1  O-PEZUOHXSCVFMTBGLRINQJWAYDK
#define  ROTORS2  F-ZOUESYDKFWPCIQXHMVBLGNJRAT
#define  ROTORS3  Z-EHRVXGAOBQUSIMZFLYNWKTPDJC
#define    UKWS   --IMETCGFRAYSQBZXWLHKDVUPOJN

static const __FlashStringHelper * WHEELSF;

const __FlashStringHelper * LOGOX1;
const __FlashStringHelper * LOGOX2;

char EffSTECKER[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

char KeyPressed = 0;
char EncodedKey = 0;

boolean SerialRead = false;

typedef struct
{
   MACHINE_TYPE machine_type;
   char         STECKER[27];
   byte         WHEELTYPE[6]; // WHEELTYPE[0] REPRESENTS ENTRY CONTACTS (ETW)
                              // WHEELTYPE[1] REPRESENTS RIGHT ROTOR  (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                              // WHEELTYPE[2] REPRESENTS MIDDLE ROTOR (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                              // WHEELTYPE[3] REPRESENTS LEFT ROTOR   (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                              // WHEELTYPE[4] REPRESENTS ADDITIONAL WHEEL (M4 ONLY, OTHERWISE USE_ROTOR_NONE)
                              // WHEELTYPE[5] REPRESENTS REFLECTOR (UKW)
   byte         WHEELPOS[4];  // WHEELPOS[0] REPRESENTS LEFTMOST LETTER, ONLY ON M4, OTHERWISE UKW ON ROCKET, D, & SWISS K
                              // WHEELPOS[1] REPRESENTS LEFTMOST LETTER ON M3
                              // WHEELPOS[2] REPRESENTS MIDDLE LETTER
                              // WHEELPOS[3] REPRESENTS RIGHTMOST LETTER
   byte         ROTORPOS[4];  // ROTORPOS[0] REPRESENTS LEFTMOST ROTOR SETTING, ONLY ON M4, OTHERWISE UKW ON ROCKET, D, & SWISS K
                              // ROTORPOS[1] REPRESENTS LEFTMOST ROTOR SETTING ON M3
                              // ROTORPOS[2] REPRESENTS MIDDLE ROTOR SETTING
                              // ROTORPOS[3] REPRESENTS RIGHTMOST ROTOR SETTING
} EnigmaData;

EnigmaData EnigmaConfigData;
EnigmaData EnigmaCurrentData;

typedef enum
{
   SPLASH_STATE = 0, OPERATE_STATE, CONFIG_STATE
}  ENIGMA_STATE;

ENIGMA_STATE enigma_state = SPLASH_STATE;

#define IS_PRESSED  true
#define NOT_PRESSED false

const int BASE_X = 11; // the x-value origin of the top-left corner of the
                       //    brass-colored rectangle containing the rotor letter
                       //    - used for rotor-relative location calculations
const int BASE_Y = 37; // the y-value origin of the top-left corner of the
                       //    brass-colored rectangle containing the rotor letter
                       //    - used for rotor-relative location calculations

const int KEY_ROW_1 = 225;
const int KEY_ROW_2 = 255;
const int KEY_ROW_3 = 285;

const int LIGHT_ROW_1 = 135;
const int LIGHT_ROW_2 = 165;
const int LIGHT_ROW_3 = 195;

const int TAPE_LENGTH = 40; // number of characters printed on tape
char tape_data[TAPE_LENGTH];

int tape_group_size = 5;
int tape_group_counter = 0;

char start_plug = 0;

const int locs_x[] =
{
    31, // A
   144, // B
    94, // C
    81, // D
    69, // E
   106, // F
   131, // G
   156, // H
   194, // I
   181, // J
   206, // K
   219, // L
   194, // M
   169, // N
   219, // O
    19, // P
    19, // Q
    94, // R
    56, // S
   119, // T
   169, // U
   119, // V
    44, // W
    69, // X
    44, // Y
   144  // Z
};



// add a character to the tape
void add_char_to_tape(char ch)
{
   for (int i = 1; i < TAPE_LENGTH; i++)
   {
      tape_data[i-1] = tape_data[i];
   }
   tape_data[TAPE_LENGTH-1] = ch;

   if (tape_group_size > 0)
   {
      tape_group_counter++;
   }
   else
   {
      tape_group_counter = 0;
   }

   if ((tape_group_size != 0) && (tape_group_counter == tape_group_size))
   {
      tape_group_counter = 0;

      for (int i = 1; i < TAPE_LENGTH; i++)
      {
         tape_data[i-1] = tape_data[i];
      }
      tape_data[TAPE_LENGTH-1] = ' ';
   }

   draw_tape();
}  // add_char_to_tape()


// add a plug wire in config
void add_config_plug(char PlugKey1, char PlugKey2)
{
   EnigmaConfigData.STECKER[PlugKey1 - 65] = PlugKey2;
   EnigmaConfigData.STECKER[PlugKey2 - 65] = PlugKey1;

   draw_config_plugboard();
}  // add_config_plug()


// handle the stecker
void calculate_stecker(void)
{
   for (byte i = 0; i < 26; i++)
   {
      EffSTECKER[i] = EnigmaCurrentData.STECKER[i];
   }

   // this is a "convenient" place to print the machine type to the serial monitor port
   //    right before printing out the contents of all of the wheels

#ifdef SERIAL_MODE
   show_machine_over_serial();
#endif
}  // calculate_stecker()


// clear the tape data
void clear_tape(void)
{
   for (int i=0; i < TAPE_LENGTH; i++)
   {
      tape_data[i] = ' ';
   }

   draw_tape();

   tape_group_counter = 0;
}  // clear_tape()


// decrement a wheel position
void decrement_wheel(int wheel)
{
   EnigmaCurrentData.WHEELPOS[wheel]--;

   if ((EnigmaCurrentData.WHEELPOS[wheel] < 'A') || (EnigmaCurrentData.WHEELPOS[wheel] > 'Z'))
   {
      EnigmaCurrentData.WHEELPOS[wheel] = 'Z';
   }

   draw_rotor_letters();
}  // decrement_wheel()


// draw the variable portions of the config display
void draw_config_display(void)
{
   // draw config machine type
   tft.fillRect(170, 85, 70, 15, ILI9341_CYAN);
   tft.setTextSize(1);
   tft.setCursor(178, 89);
   tft.setTextColor(ILI9341_BLACK);

   print_machine_type();

   draw_config_reflector();
   draw_config_tape_group_size();
   draw_config_wheels();
   draw_config_rings();
   draw_config_setup();
   draw_config_plugboard();
}  // draw_config_display()


// draw the plugboard in config
void draw_config_plugboard(void)
{
   char ch = 'A';
   if ((EnigmaConfigData.machine_type == MACHINE_TYPE_M3) || (EnigmaConfigData.machine_type == MACHINE_TYPE_M4))
   {
      tft.setTextSize(2);
      tft.setTextColor(ILI9341_BLACK);

      for (int i = 0; i < 13; i++)
      {
         if (EnigmaConfigData.STECKER[i] != ch)
         {
            tft.fillRect((i * 18) + 5, 195, 14, 18, ILI9341_MAGENTA);
            tft.fillRect((i * 18) + 5, 215, 14, 18, ILI9341_MAGENTA);
         }
         else
         {
            if (start_plug == ch)
            {
               tft.fillRect((i * 18) + 5, 195, 14, 18, MY_ILI9341_ORANGE);
            }
            else
            {
               tft.fillRect((i * 18) + 5, 195, 14, 18, ILI9341_CYAN);
            }
            tft.fillRect((i * 18) + 5, 215, 14, 18, MY_ILI9341_MEDIUM_GRAY);
         }
         tft.setCursor((i * 18) + 7, 197);
         tft.print(ch);
         tft.setCursor((i * 18) + 7, 217);
         tft.print(EnigmaConfigData.STECKER[i]);

         ch++;
      }

      for (int i = 13; i < 26; i++)
      {
         if (EnigmaConfigData.STECKER[i] != ch)
         {
            tft.fillRect(((i - 13) * 18) + 5, 240, 14, 18, ILI9341_MAGENTA);
            tft.fillRect(((i - 13) * 18) + 5, 260, 14, 18, ILI9341_MAGENTA);
         }
         else
         {
            if (start_plug == ch)
            {
               tft.fillRect(((i - 13) * 18) + 5, 240, 14, 18, MY_ILI9341_ORANGE);
            }
            else
            {
               tft.fillRect(((i - 13) * 18) + 5, 240, 14, 18, ILI9341_CYAN);
            }
            tft.fillRect(((i - 13) * 18) + 5, 260, 14, 18, MY_ILI9341_MEDIUM_GRAY);
         }
         tft.setCursor(((i - 13) * 18) + 7, 242);
         tft.print(ch);
         tft.setCursor(((i - 13) * 18) + 7, 262);
         tft.print(EnigmaConfigData.STECKER[i]);

         ch++;
      }
   }
   else
   {
      // erase plugboard area
      for (int y = 190; y < 280; y++)
      {
         tft.drawFastHLine(0, y, 240, ILI9341_BLACK);

         for (int x = random(7); x < 240; x+=random(7)+1)
         {
            int r = random(10);

            if (r < 2)
            {
               tft.drawPixel(x, y, MY_ILI9341_SHADOW_GRAY);
            }
            else
            {
               if (r < 4)
               {
                  tft.drawPixel(x, y, MY_ILI9341_DARK_GRAY);
               }
               else
               {
                  if (r < 7)
                  {
                     tft.drawPixel(x, y, MY_ILI9341_MEDIUM_GRAY);
                  }
               }
            }
         }
      }
   }
}  // draw_config_plugboard()


// draw the reflector in config
void draw_config_reflector(void)
{
   tft.fillRect(170, 105, 70, 15, ILI9341_CYAN);
   tft.setTextSize(1);
   tft.setCursor(178, 109);
   tft.setTextColor(ILI9341_BLACK);
   switch (EnigmaConfigData.WHEELTYPE[5])
   {
      case USE_ROTOR_UKWA:
      case USE_ROTOR_UKWD:
      case USE_ROTOR_UKWR:
      case USE_ROTOR_UKWS:
      {
         tft.print("    A");
      }
      break;

      case USE_ROTOR_UKWB:
      {
         tft.print("    B");
      }
      break;

      case USE_ROTOR_UKWC:
      {
         tft.print("    C");
      }
      break;

      case USE_ROTOR_UKWBD:
      {
         tft.print(" Thin B");
      }
      break;

      case USE_ROTOR_UKWCD:
      {
         tft.print(" Thin C");
      }
      break;
   }
}  // draw_config_reflector()


// draw the rings in config
void draw_config_rings(void)
{
   tft.setTextSize(1);
   tft.setTextColor(ILI9341_BLACK);

   if (EnigmaConfigData.machine_type == MACHINE_TYPE_M3)
   {
      tft.fillRect(110, 170, 25, 15, MY_ILI9341_MEDIUM_GRAY);
   }
   else
   {
      tft.fillRect(110, 170, 25, 15, ILI9341_CYAN);
   }

   tft.fillRect(140, 170, 25, 15, ILI9341_CYAN);
   tft.fillRect(170, 170, 25, 15, ILI9341_CYAN);
   tft.fillRect(200, 170, 25, 15, ILI9341_CYAN);

   if (EnigmaConfigData.machine_type != MACHINE_TYPE_M3)
   {
      tft.setCursor(117, 174);
      tft.print(EnigmaConfigData.ROTORPOS[0] / 10);
      tft.print(EnigmaConfigData.ROTORPOS[0] % 10);
   }

   tft.setCursor(147, 174);
   tft.print(EnigmaConfigData.ROTORPOS[1] / 10);
   tft.print(EnigmaConfigData.ROTORPOS[1] % 10);

   tft.setCursor(177, 174);
   tft.print(EnigmaConfigData.ROTORPOS[2] / 10);
   tft.print(EnigmaConfigData.ROTORPOS[2] % 10);

   tft.setCursor(207, 174);
   tft.print(EnigmaConfigData.ROTORPOS[3] / 10);
   tft.print(EnigmaConfigData.ROTORPOS[3] % 10);
}  // draw_config_rings()


// draw the rotor numbers in config
void draw_config_rotor_numbers(void)
{
   byte rotor;
   int base_x = 120;

   for (int i = 0; i < 4; i++, base_x += 30)
   {
      rotor = EnigmaConfigData.WHEELTYPE[4 - i];

      switch (rotor)
      {
         case USE_ROTOR_1:
         {
            tft.setCursor(base_x, 154);
            tft.print("I");
         }
         break;

         case USE_ROTOR_D1:
         {
            tft.setCursor(base_x - 3, 154);
            tft.print("ID");
         }
         break;

         case USE_ROTOR_R1:
         {
            tft.setCursor(base_x - 3, 154);
            tft.print("IR");
         }
         break;

         case USE_ROTOR_S1:
         {
            tft.setCursor(base_x - 3, 154);
            tft.print("IS");
         }
         break;

         case USE_ROTOR_2:
         {
            tft.setCursor(base_x - 3, 154);
            tft.print("II");
         }
         break;

         case USE_ROTOR_D2:
         {
            tft.setCursor(base_x - 6, 154);
            tft.print("IID");
         }
         break;

         case USE_ROTOR_R2:
         {
            tft.setCursor(base_x - 6, 154);
            tft.print("IIR");
         }
         break;

         case USE_ROTOR_S2:
         {
            tft.setCursor(base_x - 6, 154);
            tft.print("IIS");
         }
         break;

         case USE_ROTOR_3:
         {
            tft.setCursor(base_x - 6, 154);
            tft.print("III");
         }
         break;

         case USE_ROTOR_D3:
         {
            tft.setCursor(base_x - 9, 154);
            tft.print("IIID");
         }
         break;

         case USE_ROTOR_R3:
         {
            tft.setCursor(base_x - 9, 154);
            tft.print("IIIR");
         }
         break;

         case USE_ROTOR_S3:
         {
            tft.setCursor(base_x - 9, 154);
            tft.print("IIIS");
         }
         break;

         case USE_ROTOR_4:
         {
            tft.setCursor(base_x - 3, 154);
            tft.print("IV");
         }
         break;

         case USE_ROTOR_5:
         {
            tft.setCursor(base_x, 154);
            tft.print("V");
         }
         break;

         case USE_ROTOR_6:
         {
            tft.setCursor(base_x - 3, 154);
            tft.print("VI");
         }
         break;

         case USE_ROTOR_7:
         {
            tft.setCursor(base_x - 6, 154);
            tft.print("VII");
         }
         break;

         case USE_ROTOR_8:
         {
            tft.setCursor(base_x - 9, 154);
            tft.print("VIII");
         }
         break;

         case USE_ROTOR_B:
         {
            if (EnigmaConfigData.machine_type == MACHINE_TYPE_M4)
            {
               tft.setCursor(base_x, 154);
               tft.print("B");
            }
         }
         break;

         case USE_ROTOR_G:
         {
            if (EnigmaConfigData.machine_type == MACHINE_TYPE_M4)
            {
               tft.setCursor(base_x, 154);
               tft.print("G");
            }
         }
         break;
      }
   }
}  // draw_config_rotor_numbers()


// draw pre-defined setups in config
void draw_config_setup(void)
{
   tft.fillRect(80, 40, 102, 30, ILI9341_CYAN);
   tft.setTextSize(1);
   tft.setCursor(83, 51);
   tft.setTextColor(ILI9341_BLACK);

   switch (config_setup)
   {
      case CONFIG_M3_1939:
      {
         tft.print("STANDARD M3 1939");
      }
      break;

      case CONFIG_EXAMPLE_1930:
      {
         tft.print("  EXAMPLE 1930");
      }
      break;

      case CONFIG_ENIGMA_D:
      {
         tft.print("    ENIGMA D");
      }
      break;

      case CONFIG_ROCKET_K_RAILWAY:
      {
         tft.print("ROCKET K RAILWAY");
      }
      break;

      case CONFIG_SWISS_K:
      {
         tft.print("    SWISS K");
      }
      break;

      case CONFIG_TURING_1940:
      {
         tft.print("  TURING 1940");
      }
      break;

      case CONFIG_BARBAROSA_1941:
      {
         tft.print(" BARBAROSA 1941");
      }
      break;

      case CONFIG_M4_1942:
      {
         tft.print("STANDARD M4 1942");
      }
      break;

      case CONFIG_U264_1942:
      {
         tft.print("  U-264 1942");
      }
      break;

      case CONFIG_SCHARNHORST_1943:
      {
         tft.print("SCHARNHORST 1943");
      }
      break;
   }
}  // draw_config_setup()


// draw the tape groups in config
void draw_config_tape_group_size(void)
{
   tft.fillRect(170, 125, 70, 15, ILI9341_CYAN);
   tft.setTextSize(1);
   tft.setCursor(178, 129);
   tft.setTextColor(ILI9341_BLACK);

   if (tape_group_size == 0)
   {
      tft.print("  NONE");
   }
   else
   {
      tft.print("    ");
      tft.print(tape_group_size);
   }
}  // draw_config_tape_group_size()


// draw the wheels in config
void draw_config_wheels(void)
{
   if (EnigmaConfigData.machine_type != MACHINE_TYPE_M4)
   {
      tft.fillRect(110, 150, 25, 15, MY_ILI9341_MEDIUM_GRAY);
   }
   else
   {
      tft.fillRect(110, 150, 25, 15, ILI9341_CYAN);
   }
   tft.fillRect(140, 150, 25, 15, ILI9341_CYAN);
   tft.fillRect(170, 150, 25, 15, ILI9341_CYAN);
   tft.fillRect(200, 150, 25, 15, ILI9341_CYAN);

   tft.setTextSize(1);
   tft.setTextColor(ILI9341_BLACK);

   draw_config_rotor_numbers();
}  // draw_config_wheels()


// draw the entire operational display
void draw_display(void)
{
   switch (enigma_state)
   {
      case SPLASH_STATE:
      {
         erase_background();

         // draw the Enigma logo (2x size)
         const char *logobptr PROGMEM = (const char PROGMEM *)LOGOX2;
         int pixel_x, pixel_y, px, ndx = 0;

         for (byte y = 0; y < 50; y++)
         {
            for (byte x = 0; x < 15; x++)
            {
               pixel_x = 60 + x * 8;
               pixel_y = 5 + y;
               px = pgm_read_byte(logobptr + ndx);

               for (byte i = 0; i < 8; i++)
               {
                  if ((px >> (7 - i)) & 0x01)
                  {
                     tft.drawPixel(pixel_x + i, pixel_y, ILI9341_WHITE);
                  }
               }

               ndx++;
            }
         }

         tft.setTextColor(ILI9341_YELLOW);
         tft.setTextSize(2);

         tft.setCursor(15, 70);
         tft.println(TITLE);

         tft.setTextColor(ILI9341_GREEN);

         tft.setCursor(15, 100);
         tft.print(VERSION0);
         tft.println(VERSION1);
//         tft.setCursor(15, 120);
//         tft.println(VERSION2);

         tft.setTextColor(ILI9341_CYAN);
         tft.setTextSize(1);

         tft.setCursor(1, 150);
         tft.println(CREDIT1);

         tft.setTextColor(ILI9341_ORANGE);
         tft.setCursor(5, 295);
         tft.println(TAPSTART);

#ifdef SERIAL_MODE
         show_init_over_serial();
#endif
      }
      break;

      case OPERATE_STATE:
      {
         erase_background();

         tft.setTextSize(1);
         tft.setTextColor(ILI9341_GREEN);
         tft.setCursor(182, 10);
         tft.print(VERSION0);

         // draw the rotors
         for (int i = 0; i < 4; i++)
         {
            if (!((EnigmaCurrentData.machine_type == MACHINE_TYPE_M3) && (i == 0)))
            {
               tft.fillRect(BASE_X - 11 + (i * 40),  BASE_Y - 12,  36, 44, MY_ILI9341_LIGHT_GRAY);
               tft.drawRect(BASE_X -  2 + (i * 40),  BASE_Y -  1,  18, 22, ILI9341_BLACK);
               tft.drawRect(BASE_X -  3 + (i * 40),  BASE_Y -  2,  20, 24, ILI9341_BLACK);
               tft.drawRect(BASE_X -  4 + (i * 40),  BASE_Y -  3,  22, 26, MY_ILI9341_LIGHT_GRAY);
               tft.drawRect(BASE_X -  5 + (i * 40),  BASE_Y -  4,  24, 28, MY_ILI9341_LIGHT_GRAY);
               tft.drawRect(BASE_X -  6 + (i * 40),  BASE_Y -  5,  26, 30, MY_ILI9341_DARK_GRAY);
               tft.drawRect(BASE_X -  7 + (i * 40),  BASE_Y -  6,  28, 32, MY_ILI9341_DARK_GRAY);
               tft.drawRect(BASE_X -  8 + (i * 40),  BASE_Y -  7,  30, 34, MY_ILI9341_DARK_GRAY);

               tft.fillCircle(BASE_X + 6 + (i * 40), BASE_Y - 25, 12, MY_ILI9341_LIGHT_GRAY);
               tft.fillCircle(BASE_X + 6 + (i * 40), BASE_Y + 44, 12, MY_ILI9341_LIGHT_GRAY);

               for (int x = 0; x < 14; x++)
               {
                  tft.drawLine(BASE_X -  7 + (i * 40), BASE_Y - 26, BASE_X - 10 + x + (i * 40), BASE_Y - 13, MY_ILI9341_LIGHT_GRAY);
                  tft.drawLine(BASE_X + 19 + (i * 40), BASE_Y - 26, BASE_X + 10 + x + (i * 40), BASE_Y - 13, MY_ILI9341_LIGHT_GRAY);

                  tft.drawLine(BASE_X -  7 + (i * 40), BASE_Y + 45, BASE_X - 10 + x + (i * 40), BASE_Y + 32, MY_ILI9341_LIGHT_GRAY);
                  tft.drawLine(BASE_X + 19 + (i * 40), BASE_Y + 45, BASE_X + 10 + x + (i * 40), BASE_Y + 32, MY_ILI9341_LIGHT_GRAY);
               }

               tft.drawCircle(BASE_X + 6 + (i * 40), BASE_Y - 29, 6, ILI9341_BLACK);
               tft.fillCircle(BASE_X + 6 + (i * 40), BASE_Y - 29, 5, MY_ILI9341_DARK_GRAY);
               tft.fillCircle(BASE_X + 6 + (i * 40), BASE_Y - 29, 3, MY_ILI9341_MEDIUM_GRAY);
               tft.drawCircle(BASE_X + 6 + (i * 40), BASE_Y + 48, 6, ILI9341_BLACK);
               tft.fillCircle(BASE_X + 6 + (i * 40), BASE_Y + 48, 5, MY_ILI9341_DARK_GRAY);
               tft.fillCircle(BASE_X + 6 + (i * 40), BASE_Y + 48, 3, MY_ILI9341_MEDIUM_GRAY);

               tft.drawLine(BASE_X + 3 + (i * 40), BASE_Y - 29, BASE_X + 9 + (i * 40), BASE_Y - 29, ILI9341_BLACK);
               tft.drawLine(BASE_X + 3 + (i * 40), BASE_Y + 48, BASE_X + 9 + (i * 40), BASE_Y + 48, ILI9341_BLACK);

               draw_rotor_letters();
            }
         }

         // draw the Enigma logo (1x size)
         const char *logobptr PROGMEM = (const char PROGMEM *)LOGOX1;
         int pixel_x, pixel_y, px, ndx = 0;

         for (byte y = 0; y < 25; y++)
         {
            for (byte x = 0; x < 8; x++)
            {
               pixel_x = 170 + x * 8;
               pixel_y =  30 + y;
               px = pgm_read_byte(logobptr + ndx);

               for (byte i = 0; i < 8; i++)
               {
                  if ((px >> (7 - i)) & 0x01)
                  {
                     tft.drawPixel(pixel_x + i, pixel_y, ILI9341_WHITE);
                  }
               }

               ndx++;
            }
         }

         tft.drawRect(168, 65, 64, 18, MY_ILI9341_LIGHT_GRAY);
         tft.drawRect(169, 66, 62, 16, MY_ILI9341_LIGHT_GRAY);
         tft.drawRect(170, 67, 60, 14, MY_ILI9341_LIGHT_GRAY);
         tft.fillRect(171, 68, 58, 12, ILI9341_BLACK);

         tft.setTextSize(1);
         tft.setTextColor(ILI9341_GREEN);
         tft.setCursor(174, 70);

         print_machine_type();

         for (int ch = 'A'; ch <= 'Z'; ch++)
         {
            draw_lights(ch, NOT_PRESSED);
            draw_keys(ch, NOT_PRESSED);
         }

         clear_tape();

         // draw the operational plugboard
         char ch = 'A';

         if ((EnigmaConfigData.machine_type == MACHINE_TYPE_M3) || (EnigmaConfigData.machine_type == MACHINE_TYPE_M4))
         {
            tft.setTextSize(1);
            tft.setTextColor(ILI9341_BLACK);

            for (int i = 0; i < 26; i++)
            {
               if (EnigmaCurrentData.STECKER[i] != ch)
               {
                  tft.fillRect((i * 9) + 3, 300, 7, 9, ILI9341_MAGENTA);
                  tft.fillRect((i * 9) + 3, 311, 7, 9, ILI9341_MAGENTA);
               }
               else
               {
                  tft.fillRect((i * 9) + 3, 300, 7, 9, MY_ILI9341_LIGHT_GRAY);
                  tft.fillRect((i * 9) + 3, 311, 7, 9, MY_ILI9341_MEDIUM_GRAY);
               }
               tft.setCursor((i * 9) + 4, 301);
               tft.print(ch);
               tft.setCursor((i * 9) + 4, 312);
               tft.print(EnigmaCurrentData.STECKER[i]);

               ch++;
            }
         }
      }
      break;

      case CONFIG_STATE:
      {
         erase_background();

         tft.setTextSize(1);
         tft.setTextColor(ILI9341_GREEN);
         tft.setCursor(195, 10);
         tft.print(VERSION0);

         tft.setTextSize(2);
         tft.setCursor(88, 7);
         tft.setTextColor(ILI9341_GREEN);
         tft.print("CONFIG");
         tft.drawRect(75, 1, 100, 26, ILI9341_RED);
         tft.drawRect(76, 2,  98, 24, ILI9341_RED);

         tft.setTextSize(2);
         tft.setCursor(10, 85);
         tft.setTextColor(ILI9341_YELLOW);
         tft.print("Machine Type");

         tft.setTextSize(2);
         tft.setCursor(10, 105);
         tft.setTextColor(ILI9341_YELLOW);
         tft.print("Reflector");

         tft.setTextSize(2);
         tft.setCursor(10, 125);
         tft.setTextColor(ILI9341_YELLOW);
         tft.print("Tape Groups");

         tft.setTextSize(2);
         tft.setCursor(10, 150);
         tft.setTextColor(ILI9341_YELLOW);
         tft.print("Wheels");

         tft.setTextSize(2);
         tft.setCursor(10, 170);
         tft.setTextColor(ILI9341_YELLOW);
         tft.print("Rings");

         tft.setTextSize(2);
         tft.setCursor(10, 39);
         tft.setTextColor(ILI9341_YELLOW);
         tft.print("Quick");
         tft.setCursor(10, 57);
         tft.print("Setup");

         draw_config_display();

         tft.setTextSize(1);
         tft.setTextColor(ILI9341_BLACK);

         tft.fillRect(190, 40, 50, 30, MY_ILI9341_ORANGE);
         tft.drawRect(192, 42, 46, 26, ILI9341_BLACK);
         tft.setCursor(203, 51);
         tft.print("LOAD");

         tft.fillRect(30, 290, 60, 30, ILI9341_RED);
         tft.drawRect(32, 292, 56, 26, ILI9341_BLACK);
         tft.setCursor(40, 301);
         tft.print("DISCARD");

         tft.fillRect(150, 290, 60, 30, ILI9341_GREEN);
         tft.drawRect(152, 292, 56, 26, ILI9341_BLACK);
         tft.setCursor(157, 301);
         tft.print("ACTIVATE");
      }
      break;
   }
}  // draw_display()


// draw the keyboard
void draw_keys(char ch, boolean is_pressed)
{
   int key_locs_y[26] =
   {
      KEY_ROW_2, // A
      KEY_ROW_3, // B
      KEY_ROW_3, // C
      KEY_ROW_2, // D
      KEY_ROW_1, // E
      KEY_ROW_2, // F
      KEY_ROW_2, // G
      KEY_ROW_2, // H
      KEY_ROW_1, // I
      KEY_ROW_2, // J
      KEY_ROW_2, // K
      KEY_ROW_3, // L
      KEY_ROW_3, // M
      KEY_ROW_3, // N
      KEY_ROW_1, // O
      KEY_ROW_3, // P
      KEY_ROW_1, // Q
      KEY_ROW_1, // R
      KEY_ROW_2, // S
      KEY_ROW_1, // T
      KEY_ROW_1, // U
      KEY_ROW_3, // V
      KEY_ROW_1, // W
      KEY_ROW_3, // X
      KEY_ROW_3, // Y
      KEY_ROW_1  // Z
   };

   int index = ch - 'A';

   if (is_pressed)
   {
      tft.fillCircle(locs_x[index], key_locs_y[index], 9, ILI9341_RED);
      tft.drawChar(locs_x[index] - 5, key_locs_y[index] - 6, ch, ILI9341_WHITE, ILI9341_RED, 2);
   }
   else
   {
      tft.fillCircle(locs_x[index], key_locs_y[index], 9, ILI9341_BLACK);
      tft.drawChar(locs_x[index] - 5, key_locs_y[index] - 6, ch, ILI9341_WHITE, ILI9341_BLACK, 2);
   }
   tft.drawCircle(locs_x[index], key_locs_y[index], 10, MY_ILI9341_LIGHT_GRAY);
   tft.drawCircle(locs_x[index], key_locs_y[index], 11, MY_ILI9341_LIGHT_GRAY);
}  // draw_keys()


// draw the indicator lights
void draw_lights(char ch, boolean is_pressed)
{
   int light_locs_y[26] =
   {
      LIGHT_ROW_2, // A
      LIGHT_ROW_3, // B
      LIGHT_ROW_3, // C
      LIGHT_ROW_2, // D
      LIGHT_ROW_1, // E
      LIGHT_ROW_2, // F
      LIGHT_ROW_2, // G
      LIGHT_ROW_2, // H
      LIGHT_ROW_1, // I
      LIGHT_ROW_2, // J
      LIGHT_ROW_2, // K
      LIGHT_ROW_3, // L
      LIGHT_ROW_3, // M
      LIGHT_ROW_3, // N
      LIGHT_ROW_1, // O
      LIGHT_ROW_3, // P
      LIGHT_ROW_1, // Q
      LIGHT_ROW_1, // R
      LIGHT_ROW_2, // S
      LIGHT_ROW_1, // T
      LIGHT_ROW_1, // U
      LIGHT_ROW_3, // V
      LIGHT_ROW_1, // W
      LIGHT_ROW_3, // X
      LIGHT_ROW_3, // Y
      LIGHT_ROW_1  // Z
   };

   int index = ch - 'A';

   if (is_pressed)
   {
      tft.fillCircle(locs_x[index], light_locs_y[index], 9, MY_ILI9341_MEDIUM_GRAY);
      tft.drawChar(locs_x[index] - 5, light_locs_y[index] - 6, ch, ILI9341_YELLOW, MY_ILI9341_MEDIUM_GRAY, 2);
   }
   else
   {
      tft.fillCircle(locs_x[index], light_locs_y[index], 9, ILI9341_BLACK);
      tft.drawChar(locs_x[index] - 5, light_locs_y[index] - 6, ch, MY_ILI9341_SHADOW_GRAY, ILI9341_BLACK, 2);
   }
   tft.drawCircle(locs_x[index], light_locs_y[index], 10, MY_ILI9341_MEDIUM_GRAY);
   tft.drawCircle(locs_x[index], light_locs_y[index], 11, MY_ILI9341_MEDIUM_GRAY);
}  // draw_lights()


// draw the letters in the rotors
void draw_rotor_letters(void)
{
   if (EnigmaCurrentData.machine_type != MACHINE_TYPE_M3)
   {
      tft.fillRect(BASE_X,  BASE_Y,  14, 20, MY_ILI9341_BRASS);
      tft.drawChar(BASE_X + 2,  BASE_Y + 3, EnigmaCurrentData.WHEELPOS[0], ILI9341_BLACK, MY_ILI9341_BRASS, 2);
   }

   for (int i = 1; i < 4; i++)
   {
      tft.fillRect(BASE_X + (i * 40),  BASE_Y,  14, 20, MY_ILI9341_BRASS);
      tft.drawChar(BASE_X + 2 + (i * 40),  BASE_Y + 3, EnigmaCurrentData.WHEELPOS[i], ILI9341_BLACK, MY_ILI9341_BRASS, 2);
   }
}  // draw_rotor_letters


// draw the tape
void draw_tape(void)
{
   int space_counter = 0;

   while (tape_data[space_counter] == ' ')
   {
      space_counter++;
   }

   tft.fillRect(0, 100, 240, 15, MY_ILI9341_SHADOW_GRAY);

   if (space_counter == 0)
   {
      tft.fillRect(0, 100, 240, 15, MY_ILI9341_PAPER);
   }
   else
   {
      tft.fillRect((space_counter - 1) * 6, 100, 240, 15, MY_ILI9341_PAPER);
      tft.drawPixel((space_counter - 1) * 6 - 1, 102, MY_ILI9341_PAPER);
      tft.drawPixel((space_counter - 1) * 6 - 2, 103, MY_ILI9341_PAPER);
      tft.drawPixel((space_counter - 1) * 6 - 1, 105, MY_ILI9341_PAPER);
      tft.drawPixel((space_counter - 1) * 6 - 2, 106, MY_ILI9341_PAPER);
      tft.drawPixel((space_counter - 1) * 6 - 1, 108, MY_ILI9341_PAPER);
      tft.drawPixel((space_counter - 1) * 6 - 2, 109, MY_ILI9341_PAPER);
      tft.drawPixel((space_counter - 1) * 6 - 1, 111, MY_ILI9341_PAPER);
      tft.drawPixel((space_counter - 1) * 6 - 2, 112, MY_ILI9341_PAPER);
   }

   tft.setCursor(0, 104);
   tft.setTextSize(1);
   tft.setTextColor(MY_ILI9341_MEDIUM_GRAY);

   for (int i = 0; i < TAPE_LENGTH; i++)
   {
      tft.print(tape_data[i]);
   }
}  // draw_tape()


// calculate the letter translation
char encode_key(char key)
{
   const char *charptr PROGMEM = (const char PROGMEM *)WHEELSF;
   char k, k1;
   unsigned int wheeltype;

#ifdef SERIAL_MODE
   serial_monitor(0);
   serial_monitor(key);
#endif

   k = EffSTECKER[key - 'A'];

#ifdef SERIAL_MODE
   serial_monitor(k);
#endif

   for (byte i = 0; i < 6; i++)
   {
      if (EnigmaCurrentData.WHEELTYPE[i] != 0)
      {
         if ((i > 0) && (i < 5))
         {
            byte p = EnigmaCurrentData.WHEELPOS[4 - i] - (EnigmaCurrentData.ROTORPOS[4 - i] - 1);

            if (p < 'A')
            {
               p += 26;
            }

            k += (p - 'A');
         }
         else
         {
            // use WHEELPOS[0] & ROTORPOS[0] for UKW on Enigma D, Rocket K, & Swiss K
            if ((i == 5) &&
                ((EnigmaCurrentData.machine_type == MACHINE_TYPE_ENIGMA_D) ||
                 (EnigmaCurrentData.machine_type == MACHINE_TYPE_ROCKET_K) ||
                 (EnigmaCurrentData.machine_type == MACHINE_TYPE_SWISS_K)))
            {
               byte p = EnigmaCurrentData.WHEELPOS[0] - (EnigmaCurrentData.ROTORPOS[0] - 1);

               if (p < 'A')
               {
                  p += 26;
               }

               k += (p - 'A');
            }
         }


         if (k > 'Z')
         {
            k -= ('Z' + 1);
         }
         else
         {
            k -= 'A';
         }

         wheeltype = ((EnigmaCurrentData.WHEELTYPE[i] - 1) * 28) + k + 2;

         k = pgm_read_byte(charptr + wheeltype);

         if ((i > 0) && (i < 5))
         {
            byte p = EnigmaCurrentData.WHEELPOS[4 - i] - (EnigmaCurrentData.ROTORPOS[4 - i] - 1);

            if (p < 'A')
            {
               p += 26;
            }

            k -= (p - 'A');
         }
         else
         {
            // use WHEELPOS[0] & ROTORPOS[0] for UKW on Enigma D, Rocket K, & Swiss K
            if ((i == 5) &&
                ((EnigmaCurrentData.machine_type == MACHINE_TYPE_ENIGMA_D) ||
                 (EnigmaCurrentData.machine_type == MACHINE_TYPE_ROCKET_K) ||
                 (EnigmaCurrentData.machine_type == MACHINE_TYPE_SWISS_K)))
            {
               byte p = EnigmaCurrentData.WHEELPOS[0] - (EnigmaCurrentData.ROTORPOS[0] - 1);

               if (p < 'A')
               {
                  p += 26;
               }

               k -= (p - 'A');
            }
         }

         if (k < 'A')
         {
            k += 26;
         }

#ifdef SERIAL_MODE
         serial_monitor(k);
#endif
      }
   }

   // after reflector

   for (byte i = 0; i < 5; i++)
   {
      if (EnigmaCurrentData.WHEELTYPE[4 - i] != 0)
      {
         if (i < 4)
         {
            byte p = EnigmaCurrentData.WHEELPOS[i] - (EnigmaCurrentData.ROTORPOS[i] - 1);
            if (p < 'A')
            {
               p += 26;
            }

            k += (p - 'A');
         }

         if (k > 'Z')
         {
            k -= 26;
         }

         wheeltype = (EnigmaCurrentData.WHEELTYPE[4 - i] - 1) * 28;

         for (byte j = 0; j < 26; j++)
         {
            if ((pgm_read_byte(charptr + wheeltype + j + 2)) == k)
            {
               k1 = 'A' + j;
            }
         }

         k = k1;

         if (i < 4)
         {
            byte p = EnigmaCurrentData.WHEELPOS[i] - (EnigmaCurrentData.ROTORPOS[i] - 1);

            if (p < 'A')
            {
               p += 26;
            }

            k -= (p - 'A');
         }

         if (k < 'A')
         {
            k += 26;
         }

#ifdef SERIAL_MODE
         serial_monitor(k);
#endif
      }
   }

   for (byte j = 0; j < 26; j++)
   {
      if (EffSTECKER[j] == k)
      {
         k1 = 'A' + j;
      }
   }

   k = k1;

#ifdef SERIAL_MODE
   serial_monitor(k);
#endif

   return k;
}  // encode_key()


// erase the background to crinkle black/gray
void erase_background(void)
{
   for (int y = 0; y < 320; y++)
   {
      tft.drawFastHLine(0, y, 240, ILI9341_BLACK);

      for (int x = random(7); x < 240; x+=random(7)+1)
      {
         int r = random(10);

         if (r < 2)
         {
            tft.drawPixel(x, y, MY_ILI9341_SHADOW_GRAY);
         }
         else
         {
            if (r < 4)
            {
               tft.drawPixel(x, y, MY_ILI9341_DARK_GRAY);
            }
            else
            {
               if (r < 7)
               {
                  tft.drawPixel(x, y, MY_ILI9341_MEDIUM_GRAY);
               }
            }
         }
      }
   }
}  // erase_background()


// increment a wheel position
void increment_wheel(int wheel)
{
   EnigmaCurrentData.WHEELPOS[wheel]++;

   if ((EnigmaCurrentData.WHEELPOS[wheel] > 'Z') || (EnigmaCurrentData.WHEELPOS[wheel] < 'A'))
   {
      EnigmaCurrentData.WHEELPOS[wheel] = 'A';
   }

   draw_rotor_letters();
}  // increment_wheel()


// see if it is time to rotate a wheel
bool is_carry(byte wheelType, byte wheelPos)
{
   const char *charptr PROGMEM = (const char PROGMEM *)WHEELSF;
   unsigned int wheeltype = (wheelType - 1) * 28;

   byte k1 = pgm_read_byte(charptr + wheeltype);
   byte k2 = pgm_read_byte(charptr + wheeltype + 1);

   if ((wheelPos == k1) || (wheelPos == k2))
   {
      return(true);
   }
   else
   {
      return(false);
   }
}  // is_carry()


// loop forever
void loop()
{
#ifdef SERIAL_MODE
  process_serial();
#endif

  process_buttons();
}  // loop()


// move the wheels with each keypress, but before determining the resulting letter translation
void move_wheels(void)
{
   byte i = 4;
   bool carry = true;

   do
   {
      i--;

      if (carry)
      {
         EnigmaCurrentData.WHEELPOS[i]++;

         if (i > 1)
         {
            carry = is_carry(EnigmaCurrentData.WHEELTYPE[4 - i], EnigmaCurrentData.WHEELPOS[i]);
         }
         else
         {
            carry = false;
         }
      }
      else
      {
         // double stepping on second wheel
         if (i == 2)
         {
            byte w2 = EnigmaCurrentData.WHEELPOS[2] + 1;

            if (w2 > 'Z')
            {
               w2 = 'A';
            }

            if (is_carry(EnigmaCurrentData.WHEELTYPE[2], w2))
            {
               EnigmaCurrentData.WHEELPOS[2]++;

               carry = true;
            }
         }
      }

      if (EnigmaCurrentData.WHEELPOS[i] > 'Z')
      {
         EnigmaCurrentData.WHEELPOS[i] = 'A';

         carry = is_carry(EnigmaCurrentData.WHEELTYPE[4 - i], EnigmaCurrentData.WHEELPOS[i]) || carry;

         if (i == 1)
         {
            carry = false;
         }
      }
   } while (i > 0);
}  // move_wheels()


// print machine type
void print_machine_type()
{
   switch (EnigmaConfigData.machine_type)
   {
      case MACHINE_TYPE_M3:
      {
         tft.print("M3 (1939)");
      }
      break;

      case MACHINE_TYPE_M4:
      {
         tft.print("M4 (1942)");
      }
      break;

      case MACHINE_TYPE_ENIGMA_D:
      {
         tft.print("Enigma D");
      }
      break;

      case MACHINE_TYPE_ROCKET_K:
      {
         tft.print("Rocket K");
      }
      break;

      case MACHINE_TYPE_SWISS_K:
      {
         tft.print(" Swiss K");
      }
   }
}  // print_machine_type


// detect button presses
void process_buttons()
{
   boolean key_pressed = false;
   boolean button_pressed = false;
   boolean wait_for_release = false;

   char encoded_key;
   char pressed_key;

   // See if there's any touch data for us
   if (ts.bufferEmpty())
   {
      return;
   }

   // a point object holds x y and z coordinates.
   TS_Point p = ts.getPoint();

   // Scale from ~0->4000 to tft.width using the calibration #'s
   p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
   p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());

//   // show where the screen was touched (for debugging purposes)
//   tft.fillRect(0, 0, 240, 10, ILI9341_BLACK);
//   tft.setTextColor(ILI9341_WHITE);
//   tft.setTextSize(1);
//   tft.setCursor(80, 0);
//   tft.println("X: ");
//   tft.setCursor(95, 0);
//   tft.println(p.x);
//   tft.setCursor(130, 0);
//   tft.println("Y: ");
//   tft.setCursor(145, 0);
//   tft.println(p.y);

   // now, empty any buffered data
   while (! ts.bufferEmpty())
   {
      TS_Point p_discard = ts.getPoint();
   }

   switch (enigma_state)
   {
      case SPLASH_STATE:
      {
         if ((p.x >= 0) && (p.x <= 240) && (p.y >= 0) && (p.y <= 320))
         {
            button_pressed = true;
            wait_for_release = true;
         }
      }
      break;

      case OPERATE_STATE:
      {
         // click on the enigma logo or the machine type to enter config
         if ((p.x >= 160) && (p.x <= 240) && (p.y >= 25) && (p.y <= 80))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // clear the tape
         if ((p.x >= 0) && (p.x <= 240) && (p.y >= 100) && (p.y <= 115))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // click below the rotors - increment the letter
         if ((p.y >= 70) && (p.y < 95))
         {
            // click below rotor 1
            if ((p.x >= 0) && (p.x < 40))
            {
               if (EnigmaCurrentData.machine_type != MACHINE_TYPE_M3)
               {
                  button_pressed = true;
                  wait_for_release = true;
               }
            }

            // click below rotor 2
            if ((p.x >= 40) && (p.x < 80))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            // click below rotor 3
            if ((p.x >= 80) && (p.x < 120))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            // click below rotor 4
            if ((p.x >= 120) && (p.x < 160))
            {
               button_pressed = true;
               wait_for_release = true;
            }
         }

         // click above the rotors - decrement the letter
         if ((p.y >= 0) && (p.y < 25))
         {
            // click above rotor 1
            if ((p.x >= 0) && (p.x < 40))
            {
               if (EnigmaCurrentData.machine_type != MACHINE_TYPE_M3)
               {
                  button_pressed = true;
                  wait_for_release = true;
               }
            }

            // click above rotor 2
            if ((p.x >= 40) && (p.x < 80))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            // click above rotor 3
            if ((p.x >= 80) && (p.x < 120))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            // click above rotor 4
            if ((p.x >= 120) && (p.x < 160))
            {
               button_pressed = true;
               wait_for_release = true;
            }
         }

         // first row key is pressed
         if ((p.y >= 213) && (p.y <= 237))
         {
            // Q is pressed
            if ((p.x >= 7) && (p.x <= 31))
            {
               pressed_key = 'Q';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // W is pressed
            if ((p.x >= 32) && (p.x <= 56))
            {
               pressed_key = 'W';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // E is pressed
            if ((p.x >= 57) && (p.x <= 81))
            {
               pressed_key = 'E';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // R is pressed
            if ((p.x >= 82) && (p.x <= 106))
            {
               pressed_key = 'R';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // T is pressed
            if ((p.x >= 107) && (p.x <= 131))
            {
               pressed_key = 'T';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // Z is pressed
            if ((p.x >= 132) && (p.x <= 156))
            {
               pressed_key = 'Z';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // U is pressed
            if ((p.x >= 157) && (p.x <= 181))
            {
               pressed_key = 'U';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // I is pressed
            if ((p.x >= 182) && (p.x <= 206))
            {
               pressed_key = 'I';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // O is pressed
            if ((p.x >= 207) && (p.x <= 232))
            {
               pressed_key = 'O';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }
         }

         // second row key is pressed
         if ((p.y >= 243) && (p.y <= 267))
         {
            // A is pressed
            if ((p.x >= 19) && (p.x <= 43))
            {
               pressed_key = 'A';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // S is pressed
            if ((p.x >= 44) && (p.x <= 68))
            {
               pressed_key = 'S';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // D is pressed
            if ((p.x >= 69) && (p.x <= 93))
            {
               pressed_key = 'D';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // F is pressed
            if ((p.x >= 94) && (p.x <= 118))
            {
               pressed_key = 'F';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // G is pressed
            if ((p.x >= 119) && (p.x <= 143))
            {
               pressed_key = 'G';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // H is pressed
            if ((p.x >= 144) && (p.x <= 168))
            {
               pressed_key = 'H';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // J is pressed
            if ((p.x >= 169) && (p.x <= 193))
            {
               pressed_key = 'J';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // K is pressed
            if ((p.x >= 194) && (p.x <= 218))
            {
               pressed_key = 'K';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }
         }

         // third row key is pressed
         if ((p.y >= 269) && (p.y <= 293))
         {
            // P is pressed
            if ((p.x >= 7) && (p.x <= 31))
            {
               pressed_key = 'P';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // Y is pressed
            if ((p.x >= 32) && (p.x <= 56))
            {
               pressed_key = 'Y';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // X is pressed
            if ((p.x >= 57) && (p.x <= 81))
            {
               pressed_key = 'X';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // C is pressed
            if ((p.x >= 82) && (p.x <= 106))
            {
               pressed_key = 'C';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // V is pressed
            if ((p.x >= 107) && (p.x <= 131))
            {
               pressed_key = 'V';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // B is pressed
            if ((p.x >= 132) && (p.x <= 156))
            {
               pressed_key = 'B';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // N is pressed
            if ((p.x >= 157) && (p.x <= 181))
            {
               pressed_key = 'N';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // M is pressed
            if ((p.x >= 182) && (p.x <= 206))
            {
               pressed_key = 'M';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }

            // L is pressed
            if ((p.x >= 207) && (p.x <= 232))
            {
               pressed_key = 'L';

               encoded_key = process_key_inputs(pressed_key);

               key_pressed = true;
               button_pressed = true;
               wait_for_release = true;
            }
         }
      }
      break;

      case CONFIG_STATE:
      {
         // detect LOAD button
         if ((p.x >= 190) && (p.x <= 240) && (p.y >= 45) && (p.y <= 75))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect DISCARD button
         if ((p.x >= 30) && (p.x <= 90) && (p.y >= 290) && (p.y < 320))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect ACTIVATE button
         if ((p.x >= 150) && (p.x <= 210) && (p.y >= 290) && (p.y < 320))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect machine type button
         if ((p.x >= 0) && (p.x <= 240) && (p.y >= 75) && (p.y < 95))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect reflector type button
         if ((p.x >= 0) && (p.x <= 240) && (p.y >= 95) && (p.y < 115))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect tape group size button
         if ((p.x >= 0) && (p.x <= 240) && (p.y >= 115) && (p.y < 135))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect wheel4 button
         if ((p.x >= 110) && (p.x <= 135) && (p.y >= 150) && (p.y < 160))
         {
            if (EnigmaConfigData.machine_type == MACHINE_TYPE_M4)
            {
               button_pressed = true;
               wait_for_release = true;
            }
         }

         // detect wheel3 button
         if ((p.x >= 140) && (p.x <= 165) && (p.y >= 150) && (p.y < 160))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect wheel2 button
         if ((p.x >= 170) && (p.x <= 195) && (p.y >= 150) && (p.y < 160))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect wheel1 button
         if ((p.x >= 200) && (p.x <= 225) && (p.y >= 150) && (p.y < 160))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect ring4 button
         if ((p.x >= 110) && (p.x <= 135) && (p.y >= 170) && (p.y < 180))
         {
            if (EnigmaConfigData.machine_type != MACHINE_TYPE_M3)
            {
               button_pressed = true;
               wait_for_release = true;
            }
         }

         // detect ring3 button
         if ((p.x >= 140) && (p.x <= 165) && (p.y >= 170) && (p.y < 180))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect ring2 button
         if ((p.x >= 170) && (p.x <= 195) && (p.y >= 170) && (p.y < 180))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect ring1 button
         if ((p.x >= 200) && (p.x <= 225) && (p.y >= 170) && (p.y < 180))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect setup type button
         if ((p.x >= 0) && (p.x <= 180) && (p.y >= 40) && (p.y <= 70))
         {
            button_pressed = true;
            wait_for_release = true;
         }

         // detect rows of plugs
         if ((((p.y >= 195) && (p.y <= 235)) || ((p.y >= 240) && (p.y < 280))) && ((EnigmaConfigData.machine_type == MACHINE_TYPE_M3) || (EnigmaConfigData.machine_type == MACHINE_TYPE_M4)))
         {
            if ((p.x >= 5) && (p.x <= 19))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 23) && (p.x <= 37))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 41) && (p.x <= 55))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 59) && (p.x <= 73))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 77) && (p.x <= 91))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 95) && (p.x <= 109))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 113) && (p.x <= 127))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 131) && (p.x <= 145))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 149) && (p.x <= 163))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 167) && (p.x <= 181))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 185) && (p.x <= 199))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 203) && (p.x <= 217))
            {
               button_pressed = true;
               wait_for_release = true;
            }

            if ((p.x >= 221) && (p.x <= 235))
            {
               button_pressed = true;
               wait_for_release = true;
            }
         }
      }
      break;
   }

   if (button_pressed)
   {
      boolean debounce = true;

      while ((wait_for_release) && (debounce))
      {
         while (ts.touched())
         {
            TS_Point discard_p = ts.getPoint();
            delay(50);
         }

         // see if the touch has really gone away,
         // or did we just slip through the built-in debounce ??
         TS_Point p_discard = ts.getPoint();

         // if currently not being touched, then empty any buffered data
         if (! ts.touched())
         {
            while (! ts.bufferEmpty())
            {
               TS_Point p_discard = ts.getPoint();
            }

            debounce = false;
         }
      }

      if (key_pressed)
      {
         draw_keys(pressed_key, NOT_PRESSED);
         draw_lights(encoded_key, NOT_PRESSED);
      }
      else
      {
         process_button_inputs(p);
      }
   }
} // process_buttons()


// act on button presses
void process_button_inputs(TS_Point p)
{
   switch (enigma_state)
   {
      case SPLASH_STATE:
      {
         enigma_state = OPERATE_STATE;
   
         // initialize the machine as an M3 (1939) machine by default
         EnigmaCurrentData.machine_type = MACHINE_TYPE_M3;
   
         // Rotors (right-to-left)
         EnigmaCurrentData.WHEELTYPE[0] = USE_ROTOR_ETW;    // ENTRY CONTACTS
         EnigmaCurrentData.WHEELTYPE[1] = USE_ROTOR_1;      // RIGHT ROTOR  (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
         EnigmaCurrentData.WHEELTYPE[2] = USE_ROTOR_2;      // MIDDLE ROTOR (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
         EnigmaCurrentData.WHEELTYPE[3] = USE_ROTOR_3;      // LEFT ROTOR   (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
         EnigmaCurrentData.WHEELTYPE[4] = USE_ROTOR_NONE;   // ADDITIONAL WHEEL (M4 only)
         EnigmaCurrentData.WHEELTYPE[5] = USE_ROTOR_UKWB;   // REFLECTOR
   
         // Initial Rotor Positions
         EnigmaCurrentData.WHEELPOS[0] = 'A';    // LEFTMOST LETTER, ONLY ON M4
         EnigmaCurrentData.WHEELPOS[1] = 'A';    // LEFTMOST LETTER ON M3
         EnigmaCurrentData.WHEELPOS[2] = 'A';    // MIDDLE LETTER
         EnigmaCurrentData.WHEELPOS[3] = 'A';    // RIGHTMOST LETTER
   
         // Ring Settings
         EnigmaCurrentData.ROTORPOS[0] =  1;      // LEFTMOST ROTOR SETTING, ONLY ON M4
         EnigmaCurrentData.ROTORPOS[1] =  1;      // LEFTMOST ROTOR SETTING ON M3
         EnigmaCurrentData.ROTORPOS[2] =  1;      // MIDDLE ROTOR SETTING
         EnigmaCurrentData.ROTORPOS[3] =  1;      // RIGHTMOST ROTOR SETTING
   
         // Initialize stecker with no plugs
         remove_all_plugs();
   
         calculate_stecker();
   
#ifdef SERIAL_MODE
         show_rotors_over_serial();
#endif
   
         draw_display();         
      }
      break;

      case OPERATE_STATE:
      {
         // click on the enigma logo or the machine type to enter config
         if ((p.x >= 160) && (p.x <= 240) && (p.y >= 25) && (p.y <= 80))
         {
            enigma_state = CONFIG_STATE;

            EnigmaConfigData.machine_type = EnigmaCurrentData.machine_type;

            for (int i = 0; i < 27; i++)
            {
               EnigmaConfigData.STECKER[i] = EnigmaCurrentData.STECKER[i];
            }

            for (int i = 0; i < 6; i++)
            {
               EnigmaConfigData.WHEELTYPE[i] = EnigmaCurrentData.WHEELTYPE[i];
            }

            for (int i = 0; i < 4; i++)
            {
               EnigmaConfigData.WHEELPOS[i] = EnigmaCurrentData.WHEELPOS[i];
               EnigmaConfigData.ROTORPOS[i] = EnigmaCurrentData.ROTORPOS[i];
            }

            draw_display();
         }

         // clear the tape
         if ((p.x >= 0) && (p.x <= 240) && (p.y >= 100) && (p.y <= 115))
         {
            clear_tape();
         }

         // click below the rotors - increment the letter
         if ((p.y >= 70) && (p.y < 95))
         {
            // click below rotor 1
            if ((p.x >= 0) && (p.x < 40))
            {
               if (EnigmaCurrentData.machine_type != MACHINE_TYPE_M3)
               {
                  increment_wheel(0);
               }
            }

            // click below rotor 2
            if ((p.x >= 40) && (p.x < 80))
            {
               increment_wheel(1);
            }

            // click below rotor 3
            if ((p.x >= 80) && (p.x < 120))
            {
               increment_wheel(2);
            }

            // click below rotor 4
            if ((p.x >= 120) && (p.x < 160))
            {
               increment_wheel(3);
            }
         }

         // click above the rotors - decrement the letter
         if ((p.y >= 0) && (p.y < 25))
         {
            // click above rotor 1
            if ((p.x >= 0) && (p.x < 40))
            {
               if (EnigmaCurrentData.machine_type != MACHINE_TYPE_M3)
               {
                  decrement_wheel(0);
               }
            }

            // click above rotor 2
            if ((p.x >= 40) && (p.x < 80))
            {
               decrement_wheel(1);
            }

            // click above rotor 3
            if ((p.x >= 80) && (p.x < 120))
            {
               decrement_wheel(2);
            }

            // click above rotor 4
            if ((p.x >= 120) && (p.x < 160))
            {
               decrement_wheel(3);
            }
         }
      }
      break;

      case CONFIG_STATE:
      {
         // process LOAD button
         if ((p.x >= 190) && (p.x <= 240) && (p.y >= 40) && (p.y <= 70))
         {
            start_plug = 0;  // just in case a plug activity was in progress

            // initialize defaults (unless overriden)            
            tape_group_size = 5;
            EnigmaConfigData.WHEELTYPE[4] = USE_ROTOR_NONE;   // ADDITIONAL WHEEL (M4 only)

            // Initialize stecker with no plugs
            remove_all_plugs();

            switch (config_setup)
            {
               case CONFIG_M3_1939:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_M3;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETW;
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_1;
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_2;
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_3;
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWB;

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'A';
                  EnigmaConfigData.WHEELPOS[1] = 'A';
                  EnigmaConfigData.WHEELPOS[2] = 'A';
                  EnigmaConfigData.WHEELPOS[3] = 'A';

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] =  1;
                  EnigmaConfigData.ROTORPOS[1] =  1;
                  EnigmaConfigData.ROTORPOS[2] =  1;
                  EnigmaConfigData.ROTORPOS[3] =  1;
               }
               break;

               case CONFIG_EXAMPLE_1930:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_M3;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETW;
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_3;
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_1;
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_2;
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWA;

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'A';
                  EnigmaConfigData.WHEELPOS[1] = 'A';
                  EnigmaConfigData.WHEELPOS[2] = 'B';
                  EnigmaConfigData.WHEELPOS[3] = 'L';

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] =  1;
                  EnigmaConfigData.ROTORPOS[1] = 24;
                  EnigmaConfigData.ROTORPOS[2] = 13;
                  EnigmaConfigData.ROTORPOS[3] = 22;

                  add_config_plug('A', 'M');
                  add_config_plug('F', 'I');
                  add_config_plug('N', 'V');
                  add_config_plug('P', 'S');
                  add_config_plug('T', 'U');
                  add_config_plug('W', 'Z');
               }
               break;

               case CONFIG_ENIGMA_D:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_ENIGMA_D;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETWD;   // ENTRY CONTACTS
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_D1;     // RIGHT ROTOR  (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_D2;     // MIDDLE ROTOR (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_D3;     // LEFT ROTOR   (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWD;   // REFLECTOR

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'A';    // LEFTMOST LETTER ON M4
                  EnigmaConfigData.WHEELPOS[1] = 'A';    // LEFTMOST LETTER ON M3
                  EnigmaConfigData.WHEELPOS[2] = 'A';    // MIDDLE LETTER
                  EnigmaConfigData.WHEELPOS[3] = 'A';    // RIGHTMOST LETTER

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] =  1;      // LEFTMOST ROTOR SETTING ON M4
                  EnigmaConfigData.ROTORPOS[1] =  1;      // LEFTMOST ROTOR SETTING ON M3
                  EnigmaConfigData.ROTORPOS[2] =  1;      // MIDDLE ROTOR SETTING
                  EnigmaConfigData.ROTORPOS[3] =  1;      // RIGHTMOST ROTOR SETTING
               }
               break;

               case CONFIG_ROCKET_K_RAILWAY:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_ROCKET_K;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETWR;   // ENTRY CONTACTS
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_R1;     // RIGHT ROTOR  (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_R2;     // MIDDLE ROTOR (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_R3;     // LEFT ROTOR   (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWR;   // REFLECTOR

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'A';    // LEFTMOST LETTER ON M4
                  EnigmaConfigData.WHEELPOS[1] = 'A';    // LEFTMOST LETTER ON M3
                  EnigmaConfigData.WHEELPOS[2] = 'A';    // MIDDLE LETTER
                  EnigmaConfigData.WHEELPOS[3] = 'A';    // RIGHTMOST LETTER

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] =  1;      // LEFTMOST ROTOR SETTING ON M4
                  EnigmaConfigData.ROTORPOS[1] =  1;      // LEFTMOST ROTOR SETTING ON M3
                  EnigmaConfigData.ROTORPOS[2] =  1;      // MIDDLE ROTOR SETTING
                  EnigmaConfigData.ROTORPOS[3] =  1;      // RIGHTMOST ROTOR SETTING
               }
               break;

               case CONFIG_SWISS_K:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_SWISS_K;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETWS;   // ENTRY CONTACTS
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_S1;     // RIGHT ROTOR  (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_S2;     // MIDDLE ROTOR (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_S3;     // LEFT ROTOR   (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWS;   // REFLECTOR

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'A';    // LEFTMOST LETTER ON M4
                  EnigmaConfigData.WHEELPOS[1] = 'A';    // LEFTMOST LETTER ON M3
                  EnigmaConfigData.WHEELPOS[2] = 'A';    // MIDDLE LETTER
                  EnigmaConfigData.WHEELPOS[3] = 'A';    // RIGHTMOST LETTER

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] =  1;      // LEFTMOST ROTOR SETTING ON M4
                  EnigmaConfigData.ROTORPOS[1] =  1;      // LEFTMOST ROTOR SETTING ON M3
                  EnigmaConfigData.ROTORPOS[2] =  1;      // MIDDLE ROTOR SETTING
                  EnigmaConfigData.ROTORPOS[3] =  1;      // RIGHTMOST ROTOR SETTING
               }
               break;

               case CONFIG_TURING_1940:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_ROCKET_K;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETWR;    // ENTRY CONTACTS
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_R2;      // RIGHT ROTOR  (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_R1;      // MIDDLE ROTOR (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_R3;      // LEFT ROTOR   (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWR;    // REFLECTOR

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'J';    // LEFTMOST LETTER ON M4
                  EnigmaConfigData.WHEELPOS[1] = 'E';    // LEFTMOST LETTER ON M3
                  EnigmaConfigData.WHEELPOS[2] = 'Z';    // MIDDLE LETTER
                  EnigmaConfigData.WHEELPOS[3] = 'A';    // RIGHTMOST LETTER

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] = 26;      // LEFTMOST ROTOR SETTING ON M4
                  EnigmaConfigData.ROTORPOS[1] = 17;      // LEFTMOST ROTOR SETTING ON M3
                  EnigmaConfigData.ROTORPOS[2] = 16;      // MIDDLE ROTOR SETTING
                  EnigmaConfigData.ROTORPOS[3] = 13;      // RIGHTMOST ROTOR SETTING
               }
               break;

               case CONFIG_BARBAROSA_1941:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_M3;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETW;
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_5;
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_4;
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_2;
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWB;

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'A';
                  EnigmaConfigData.WHEELPOS[1] = 'B';
                  EnigmaConfigData.WHEELPOS[2] = 'L';
                  EnigmaConfigData.WHEELPOS[3] = 'A';

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] = 1;
                  EnigmaConfigData.ROTORPOS[1] = 2;
                  EnigmaConfigData.ROTORPOS[2] = 21;
                  EnigmaConfigData.ROTORPOS[3] = 12;

                  add_config_plug('A', 'V');
                  add_config_plug('B', 'S');
                  add_config_plug('C', 'G');
                  add_config_plug('D', 'L');
                  add_config_plug('F', 'U');
                  add_config_plug('H', 'Z');
                  add_config_plug('I', 'N');
                  add_config_plug('K', 'M');
                  add_config_plug('O', 'W');
                  add_config_plug('R', 'X');
               }
               break;

               case CONFIG_M4_1942:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_M4;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETW;    // ENTRY CONTACTS
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_1;      // RIGHT ROTOR  (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_2;      // MIDDLE ROTOR (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_3;      // LEFT ROTOR   (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[4] = USE_ROTOR_B;      // ADDITIONAL WHEEL (M4 only)
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWBD;  // REFLECTOR

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'A';    // LEFTMOST LETTER ON M4
                  EnigmaConfigData.WHEELPOS[1] = 'A';    // LEFTMOST LETTER ON M3
                  EnigmaConfigData.WHEELPOS[2] = 'A';    // MIDDLE LETTER
                  EnigmaConfigData.WHEELPOS[3] = 'A';    // RIGHTMOST LETTER

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] =  1;      // LEFTMOST ROTOR SETTING ON M4
                  EnigmaConfigData.ROTORPOS[1] =  1;      // LEFTMOST ROTOR SETTING ON M3
                  EnigmaConfigData.ROTORPOS[2] =  1;      // MIDDLE ROTOR SETTING
                  EnigmaConfigData.ROTORPOS[3] =  1;      // RIGHTMOST ROTOR SETTING

                  tape_group_size = 4;
               }
               break;

               case CONFIG_U264_1942:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_M4;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETW;    // ENTRY CONTACTS
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_1;      // RIGHT ROTOR  (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_4;      // MIDDLE ROTOR (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_2;      // LEFT ROTOR   (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[4] = USE_ROTOR_B;      // ADDITIONAL WHEEL (M4 only)
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWBD;  // REFLECTOR

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'V';    // LEFTMOST LETTER ON M4
                  EnigmaConfigData.WHEELPOS[1] = 'J';    // LEFTMOST LETTER ON M3
                  EnigmaConfigData.WHEELPOS[2] = 'N';    // MIDDLE LETTER
                  EnigmaConfigData.WHEELPOS[3] = 'A';    // RIGHTMOST LETTER

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] =  1;      // LEFTMOST ROTOR SETTING ON M4
                  EnigmaConfigData.ROTORPOS[1] =  1;      // LEFTMOST ROTOR SETTING ON M3
                  EnigmaConfigData.ROTORPOS[2] =  1;      // MIDDLE ROTOR SETTING
                  EnigmaConfigData.ROTORPOS[3] =  22;      // RIGHTMOST ROTOR SETTING

                  add_config_plug('A', 'T');
                  add_config_plug('B', 'L');
                  add_config_plug('D', 'F');
                  add_config_plug('G', 'J');
                  add_config_plug('H', 'M');
                  add_config_plug('N', 'W');
                  add_config_plug('O', 'P');
                  add_config_plug('Q', 'Y');
                  add_config_plug('R', 'Z');
                  add_config_plug('V', 'X');

                  tape_group_size = 4;
               }
               break;

               case CONFIG_SCHARNHORST_1943:
               {
                  EnigmaConfigData.machine_type = MACHINE_TYPE_M3;

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETW;    // ENTRY CONTACTS
                  EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_8;      // RIGHT ROTOR  (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_6;      // MIDDLE ROTOR (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_3;      // LEFT ROTOR   (NOTE THE REVERSED ORDER VS THE OTHER ARRAYS !!)
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWB;   // REFLECTOR

                  // Initial Rotor Positions
                  EnigmaConfigData.WHEELPOS[0] = 'A';    // LEFTMOST LETTER ON M4
                  EnigmaConfigData.WHEELPOS[1] = 'U';    // LEFTMOST LETTER ON M3
                  EnigmaConfigData.WHEELPOS[2] = 'Z';    // MIDDLE LETTER
                  EnigmaConfigData.WHEELPOS[3] = 'V';    // RIGHTMOST LETTER

                  // Ring Settings
                  EnigmaConfigData.ROTORPOS[0] =  1;      // LEFTMOST ROTOR SETTING ON M4
                  EnigmaConfigData.ROTORPOS[1] =  1;      // LEFTMOST ROTOR SETTING ON M3
                  EnigmaConfigData.ROTORPOS[2] =  8;      // MIDDLE ROTOR SETTING
                  EnigmaConfigData.ROTORPOS[3] = 13;      // RIGHTMOST ROTOR SETTING

                  add_config_plug('A', 'N');
                  add_config_plug('E', 'Z');
                  add_config_plug('H', 'K');
                  add_config_plug('I', 'J');
                  add_config_plug('L', 'R');
                  add_config_plug('M', 'Q');
                  add_config_plug('O', 'T');
                  add_config_plug('P', 'V');
                  add_config_plug('S', 'W');
                  add_config_plug('U', 'X');
               }
               break;
            }

            // redraw the config display for each config change
            draw_config_display();                  
         }

         // process DISCARD button
         if ((p.x >= 30) && (p.x <= 90) && (p.y >= 290) && (p.y < 320))
         {
            enigma_state = OPERATE_STATE;

#ifdef SERIAL_MODE
            show_rotors_over_serial();
#endif

            draw_display();
         }

         // process ACTIVATE button
         if ((p.x >= 150) && (p.x <= 210) && (p.y >= 290) && (p.y < 320))
         {
            for (int i = 0; i < 6; i++)
            {
               EnigmaCurrentData.WHEELTYPE[i] = EnigmaConfigData.WHEELTYPE[i];
            }

            for (int i = 0; i < 4; i++)
            {
               EnigmaCurrentData.WHEELPOS[i] = EnigmaConfigData.WHEELPOS[i];
               EnigmaCurrentData.ROTORPOS[i] = EnigmaConfigData.ROTORPOS[i];
            }

            start_plug = 0;  // just in case a plug activity was in progress

            enigma_state = OPERATE_STATE;

            if (EnigmaConfigData.machine_type != EnigmaCurrentData.machine_type)
            {
               EnigmaCurrentData.machine_type = EnigmaConfigData.machine_type;
            }

            for (int i = 0; i < 27; i++)
            {
               EnigmaCurrentData.STECKER[i] = EnigmaConfigData.STECKER[i];
            }

            calculate_stecker();

#ifdef SERIAL_MODE
            show_rotors_over_serial();
#endif

            draw_display();
         }

         // process machine type button
         if ((p.x >= 0) && (p.x <= 240) && (p.y >= 75) && (p.y < 95))
         {
            if (EnigmaConfigData.machine_type != MACHINE_TYPE_SWISS_K)
            {
               EnigmaConfigData.machine_type = (MACHINE_TYPE)(((int)EnigmaConfigData.machine_type) + 1);
            }
            else
            {
               EnigmaConfigData.machine_type = MACHINE_TYPE_M3;
            }

            // common to all (unless overriden)
            EnigmaConfigData.WHEELPOS[0] = 'A';
            EnigmaConfigData.ROTORPOS[0] = 1;
            EnigmaConfigData.WHEELTYPE[4] = USE_ROTOR_NONE;

            switch (EnigmaConfigData.machine_type)
            {
               case MACHINE_TYPE_M3:
               {
                  if ((EnigmaConfigData.WHEELTYPE[5] != USE_ROTOR_UKWA) &&
                      (EnigmaConfigData.WHEELTYPE[5] != USE_ROTOR_UKWB) &&
                      (EnigmaConfigData.WHEELTYPE[5] != USE_ROTOR_UKWC))
                  {
                     EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWA;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_1) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_2) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_3))
                  {
                     EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_1;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_1) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_2) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_3))
                  {
                     EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_2;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_1) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_2) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_3))
                  {
                     EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_3;
                  }

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETW;
               }
               break;

               case MACHINE_TYPE_M4:
               {
                  if ((EnigmaConfigData.WHEELTYPE[5] != USE_ROTOR_UKWBD) &&
                      (EnigmaConfigData.WHEELTYPE[5] != USE_ROTOR_UKWCD))
                  {
                     EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWBD;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[4] != USE_ROTOR_B) &&
                      (EnigmaConfigData.WHEELTYPE[4] != USE_ROTOR_G))
                  {
                     EnigmaConfigData.WHEELTYPE[4] = USE_ROTOR_B;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_1) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_2) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_3))
                  {
                     EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_1;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_1) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_2) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_3))
                  {
                     EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_2;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_1) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_2) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_3))
                  {
                     EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_3;
                  }

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETW;
               }
               break;

               case MACHINE_TYPE_ENIGMA_D:
               {
                  if (EnigmaConfigData.WHEELTYPE[5] != USE_ROTOR_UKWD)
                  {
                     EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWD;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_D1) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_D2) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_D3))
                  {
                     EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_D1;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_D1) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_D2) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_D3))
                  {
                     EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_D2;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_D1) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_D2) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_D3))
                  {
                     EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_D3;
                  }

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETWD;
               }
               break;

               case MACHINE_TYPE_ROCKET_K:
               {
                  if (EnigmaConfigData.WHEELTYPE[5] != USE_ROTOR_UKWR)
                  {
                     EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWR;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_R1) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_R2) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_R3))
                  {
                     EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_R1;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_R1) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_R2) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_R3))
                  {
                     EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_R2;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_R1) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_R2) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_R3))
                  {
                     EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_R3;
                  }

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETWR;
               }
               break;

               case MACHINE_TYPE_SWISS_K:
               {
                  if (EnigmaConfigData.WHEELTYPE[5] != USE_ROTOR_UKWS)
                  {
                     EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWS;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_S1) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_S2) &&
                      (EnigmaConfigData.WHEELTYPE[3] != USE_ROTOR_S3))
                  {
                     EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_S1;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_S1) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_S2) &&
                      (EnigmaConfigData.WHEELTYPE[2] != USE_ROTOR_S3))
                  {
                     EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_S2;
                  }

                  if ((EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_S1) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_S2) &&
                      (EnigmaConfigData.WHEELTYPE[1] != USE_ROTOR_S3))
                  {
                     EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_S3;
                  }

                  EnigmaConfigData.WHEELTYPE[0] = USE_ROTOR_ETWS;
               }
               break;
            }

            draw_config_display();
         }

         // process reflector type button
         if ((p.x >= 0) && (p.x <= 240) && (p.y >= 95) && (p.y < 115))
         {
            if (EnigmaConfigData.machine_type == MACHINE_TYPE_M3)
            {
               if (EnigmaConfigData.WHEELTYPE[5] == USE_ROTOR_UKWA)
               {
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWB;
               }
               else
               {
                  if (EnigmaConfigData.WHEELTYPE[5] == USE_ROTOR_UKWB)
                  {
                     EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWC;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWA;
                  }
               }
            }
            else
            {
               if (EnigmaConfigData.machine_type == MACHINE_TYPE_M4)
               {
                  if (EnigmaConfigData.WHEELTYPE[5] == USE_ROTOR_UKWBD)
                  {
                     EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWCD;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWBD;
                  }
               }
               else
               {
                  EnigmaConfigData.WHEELTYPE[5] = USE_ROTOR_UKWD;
               }
            }

            draw_config_reflector();            
         }

         // process tape group size button
         if ((p.x >= 0) && (p.x <= 240) && (p.y >= 115) && (p.y < 135))
         {
            switch (tape_group_size)
            {
               case 0:
               {
                  tape_group_size = 3;
               }
               break;

               case 3:
               case 4:
               case 5:
               {
                  tape_group_size += 1;
               }
               break;

               default:
               {
                  tape_group_size = 0;
               }
               break;
            }

            draw_config_tape_group_size();
         }

         // process wheel4 button
         if ((p.x >= 110) && (p.x <= 135) && (p.y >= 150) && (p.y < 160))
         {
            if (EnigmaConfigData.machine_type == MACHINE_TYPE_M4)
            {
               if (EnigmaConfigData.WHEELTYPE[4] == USE_ROTOR_B)
               {
                  EnigmaConfigData.WHEELTYPE[4] = USE_ROTOR_G;
               }
               else
               {
                  EnigmaConfigData.WHEELTYPE[4] = USE_ROTOR_B;
               }

               draw_config_wheels();
            }
         }

         // process wheel3 button
         if ((p.x >= 140) && (p.x <= 165) && (p.y >= 150) && (p.y < 160))
         {
            switch (EnigmaConfigData.machine_type)
            {
               case MACHINE_TYPE_ENIGMA_D:
               {
                  if (EnigmaConfigData.WHEELTYPE[3] < USE_ROTOR_D3)
                  {
                     EnigmaConfigData.WHEELTYPE[3]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_D1;
                  }
               }
               break;

               case MACHINE_TYPE_ROCKET_K:
               {
                  if (EnigmaConfigData.WHEELTYPE[3] < USE_ROTOR_R3)
                  {
                     EnigmaConfigData.WHEELTYPE[3]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_R1;
                  }
               }
               break;

               case MACHINE_TYPE_SWISS_K:
               {
                  if (EnigmaConfigData.WHEELTYPE[3] < USE_ROTOR_S3)
                  {
                     EnigmaConfigData.WHEELTYPE[3]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_S1;
                  }
               }
               break;

               default:
               {
                  if (EnigmaConfigData.WHEELTYPE[3] < USE_ROTOR_8)
                  {
                     EnigmaConfigData.WHEELTYPE[3]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[3] = USE_ROTOR_1;
                  }
               }
               break;
            }

            draw_config_wheels();
         }

         // process wheel2 button
         if ((p.x >= 170) && (p.x <= 195) && (p.y >= 150) && (p.y < 160))
         {
            switch (EnigmaConfigData.machine_type)
            {
               case MACHINE_TYPE_ENIGMA_D:
               {
                  if (EnigmaConfigData.WHEELTYPE[2] < USE_ROTOR_D3)
                  {
                     EnigmaConfigData.WHEELTYPE[2]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_D1;
                  }
               }
               break;

               case MACHINE_TYPE_ROCKET_K:
               {
                  if (EnigmaConfigData.WHEELTYPE[2] < USE_ROTOR_R3)
                  {
                     EnigmaConfigData.WHEELTYPE[2]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_R1;
                  }
               }
               break;

               case MACHINE_TYPE_SWISS_K:
               {
                  if (EnigmaConfigData.WHEELTYPE[2] < USE_ROTOR_S3)
                  {
                     EnigmaConfigData.WHEELTYPE[2]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_S1;
                  }
               }
               break;

               default:
               {
                  if (EnigmaConfigData.WHEELTYPE[2] < USE_ROTOR_8)
                  {
                     EnigmaConfigData.WHEELTYPE[2]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[2] = USE_ROTOR_1;
                  }
               }
               break;
            }

            draw_config_wheels();
         }

         // process wheel button
         if ((p.x >= 200) && (p.x <= 225) && (p.y >= 150) && (p.y < 160))
         {
            switch (EnigmaConfigData.machine_type)
            {
               case MACHINE_TYPE_ENIGMA_D:
               {
                  if (EnigmaConfigData.WHEELTYPE[1] < USE_ROTOR_D3)
                  {
                     EnigmaConfigData.WHEELTYPE[1]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_D1;
                  }
               }
               break;

               case MACHINE_TYPE_ROCKET_K:
               {
                  if (EnigmaConfigData.WHEELTYPE[1] < USE_ROTOR_R3)
                  {
                     EnigmaConfigData.WHEELTYPE[1]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_R1;
                  }
               }
               break;

               case MACHINE_TYPE_SWISS_K:
               {
                  if (EnigmaConfigData.WHEELTYPE[1] < USE_ROTOR_S3)
                  {
                     EnigmaConfigData.WHEELTYPE[1]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_S1;
                  }
               }
               break;

               default:
               {
                  if (EnigmaConfigData.WHEELTYPE[1] < USE_ROTOR_8)
                  {
                     EnigmaConfigData.WHEELTYPE[1]++;
                  }
                  else
                  {
                     EnigmaConfigData.WHEELTYPE[1] = USE_ROTOR_1;
                  }
               }
               break;
            }

            draw_config_wheels();
         }

         // process ring4 button
         if ((p.x >= 110) && (p.x <= 135) && (p.y >= 170) && (p.y < 180))
         {
            if (EnigmaConfigData.machine_type != MACHINE_TYPE_M3)
            {
               if (EnigmaConfigData.ROTORPOS[0] < 26)
               {
                  EnigmaConfigData.ROTORPOS[0]++;
               }
               else
               {
                  EnigmaConfigData.ROTORPOS[0] = 1;
               }

               draw_config_rings();
            }
         }

         // process ring3 button
         if ((p.x >= 140) && (p.x <= 165) && (p.y >= 170) && (p.y < 180))
         {
            if (EnigmaConfigData.ROTORPOS[1] < 26)
            {
               EnigmaConfigData.ROTORPOS[1]++;
            }
            else
            {
               EnigmaConfigData.ROTORPOS[1] = 1;
            }

            draw_config_rings();
         }

         // process ring2 button
         if ((p.x >= 170) && (p.x <= 195) && (p.y >= 170) && (p.y < 180))
         {
            if (EnigmaConfigData.ROTORPOS[2] < 26)
            {
               EnigmaConfigData.ROTORPOS[2]++;
            }
            else
            {
               EnigmaConfigData.ROTORPOS[2] = 1;
            }

            draw_config_rings();
         }

         // process ring1 button
         if ((p.x >= 200) && (p.x <= 225) && (p.y >= 170) && (p.y < 180))
         {
            if (EnigmaConfigData.ROTORPOS[3] < 26)
            {
               EnigmaConfigData.ROTORPOS[3]++;
            }
            else
            {
               EnigmaConfigData.ROTORPOS[3] = 1;
            }

            draw_config_rings();
         }

         // process setup type button
         if ((p.x >= 0) && (p.x <= 180) && (p.y >= 40) && (p.y <= 70))
         {
            config_setup++;

            if (config_setup > CONFIG_SCHARNHORST_1943)
            {
               config_setup = CONFIG_M3_1939;
            }

            draw_config_setup();
         }

         // detect top row of plugs
         if ((p.y >= 195) && (p.y <= 235) && ((EnigmaConfigData.machine_type == MACHINE_TYPE_M3) || (EnigmaConfigData.machine_type == MACHINE_TYPE_M4)))
         {
            // process plug A button
            if ((p.x >= 5) && (p.x <= 19))
            {
               process_plug_inputs('A');
            }

            if ((p.x >= 23) && (p.x <= 37))
            {
               process_plug_inputs('B');
            }

            if ((p.x >= 41) && (p.x <= 55))
            {
               process_plug_inputs('C');
            }

            if ((p.x >= 59) && (p.x <= 73))
            {
               process_plug_inputs('D');
            }

            if ((p.x >= 77) && (p.x <= 91))
            {
               process_plug_inputs('E');
            }

            if ((p.x >= 95) && (p.x <= 109))
            {
               process_plug_inputs('F');
            }

            if ((p.x >= 113) && (p.x <= 127))
            {
               process_plug_inputs('G');
            }

            if ((p.x >= 131) && (p.x <= 145))
            {
               process_plug_inputs('H');
            }

            if ((p.x >= 149) && (p.x <= 163))
            {
               process_plug_inputs('I');
            }

            if ((p.x >= 167) && (p.x <= 181))
            {
               process_plug_inputs('J');
            }

            if ((p.x >= 185) && (p.x <= 199))
            {
               process_plug_inputs('K');
            }

            if ((p.x >= 203) && (p.x <= 217))
            {
               process_plug_inputs('L');
            }

            if ((p.x >= 221) && (p.x <= 235))
            {
               process_plug_inputs('M');
            }
         }

         // detect second row of plugs
         if ((p.y >= 240) && (p.y <= 280) && ((EnigmaConfigData.machine_type == MACHINE_TYPE_M3) || (EnigmaConfigData.machine_type == MACHINE_TYPE_M4)))
         {
            if ((p.x >= 5) && (p.x <= 19))
            {
               process_plug_inputs('N');
            }

            if ((p.x >= 23) && (p.x <= 37))
            {
               process_plug_inputs('O');
            }

            if ((p.x >= 41) && (p.x <= 55))
            {
               process_plug_inputs('P');
            }

            if ((p.x >= 59) && (p.x <= 73))
            {
               process_plug_inputs('Q');
            }

            if ((p.x >= 77) && (p.x <= 91))
            {
               process_plug_inputs('R');
            }

            if ((p.x >= 95) && (p.x <= 109))
            {
               process_plug_inputs('S');
            }

            if ((p.x >= 113) && (p.x <= 127))
            {
               process_plug_inputs('T');
            }

            if ((p.x >= 131) && (p.x <= 145))
            {
               process_plug_inputs('U');
            }

            if ((p.x >= 149) && (p.x <= 163))
            {
               process_plug_inputs('V');
            }

            if ((p.x >= 167) && (p.x <= 181))
            {
               process_plug_inputs('W');
            }

            if ((p.x >= 185) && (p.x <= 199))
            {
               process_plug_inputs('X');
            }

            if ((p.x >= 203) && (p.x <= 217))
            {
               process_plug_inputs('Y');
            }

            if ((p.x >= 221) && (p.x <= 235))
            {
               process_plug_inputs('Z');
            }
         }
      }
      break;
   }
}  // process_button_inputs()


// act on keypresses
char process_key_inputs(char pressed_key)
{
   char encoded_key;

   draw_keys(pressed_key, IS_PRESSED);

   move_wheels();

   draw_rotor_letters();

   encoded_key = encode_key(pressed_key);

   add_char_to_tape(encoded_key);

   draw_lights(encoded_key, IS_PRESSED);

   return(encoded_key);
}  // process_key_inputs()


// act on plug button inputs
void process_plug_inputs(char plug_char)
{
   if (EnigmaConfigData.STECKER[plug_char - 65] != plug_char)
   {
      add_config_plug(EnigmaConfigData.STECKER[plug_char - 65], EnigmaConfigData.STECKER[plug_char - 65]);
      add_config_plug(plug_char, plug_char);

      if (start_plug != 0)
      {
         add_config_plug(plug_char, start_plug);
         start_plug = 0;
      }
   }
   else
   {
      if (start_plug == 0)
      {
         start_plug = plug_char;

         draw_config_plugboard();
      }
      else
      {
         if (start_plug == plug_char)
         {
            start_plug = 0;

            draw_config_plugboard();
         }
         else
         {
            add_config_plug(plug_char, start_plug);
            start_plug = 0;
         }
      }
   }
}  // process_plug_inputs


// process serial inputs & outputs
void process_serial(void)
{
   if (enigma_state == OPERATE_STATE)
   {
      if (Serial.available())
      {
         KeyPressed = Serial.read();

         KeyPressed = KeyPressed & (255 - 32);

         if ((KeyPressed > 'A' - 1) && (KeyPressed < 'Z' + 1))
         {
            SerialRead = true;

            move_wheels();

            draw_rotor_letters();

            draw_keys(KeyPressed, IS_PRESSED);

            EncodedKey = encode_key(KeyPressed);

            add_char_to_tape(EncodedKey);

            draw_lights(EncodedKey, IS_PRESSED);

            delay(500);

            draw_keys(KeyPressed, NOT_PRESSED);

            draw_lights(EncodedKey, NOT_PRESSED);
         }
      }

      if ((SerialRead) && (!Serial.available()))
      {
         SerialRead = false;
      }
   }
}  // process_serial()


// take out all plug wires at once
void remove_all_plugs(void)
{
   strcpy(EnigmaCurrentData.STECKER, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
   strcpy(EnigmaConfigData.STECKER, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}  // remove_all_plugs()


// send the current machine internal state out to serial
void serial_monitor(char k)
{
   static byte SerialMonitorStepIndex;

   if (k == 0)
   {
      SerialMonitorStepIndex = 0;
   }
   else
   {
      SerialMonitorStepIndex++;

      // unless this is an Enigma M4 (4-wheel machine), skip R4
      if ((EnigmaCurrentData.machine_type != MACHINE_TYPE_M4) && ((SerialMonitorStepIndex == 6) || (SerialMonitorStepIndex == 8)))
      {
         SerialMonitorStepIndex++;
      }

      Serial.print(k);

      switch (SerialMonitorStepIndex)
      {
         case 1:
         case 13:
         {
            Serial.print(F(" > Stecker > "));
         }
         break;

         case 2:
         case 12:
         {
            Serial.print(F(" > ETW > "));
         }
         break;

         case 3:
         case 11:
         {
            Serial.print(F(" > R1 > "));
         }
         break;

         case 4:
         case 10:
         {
            Serial.print(F(" > R2 > "));
         }
         break;

         case 5:
         case 9:
         {
            Serial.print(F(" > R3 > "));
         }
         break;

         case 6:
         case 8:
         {
            Serial.print(F(" > R4 > "));
         }
         break;

         case 7:
         {
            Serial.print(F(" > UKW > "));
         }
         break;

         default:
         {
            Serial.println("");
         }
      }
   }
}  // serial_monitor()


// initialize everything
void setup(void)
{
   Serial.begin(9600);

   delay(500);
   tft.begin();  // init TFT library
   delay(100);
   ts.begin();   // init TouchScreen library
   delay(100);

   analogReference(DEFAULT);

   WHEELSF = F(STR(ETW) STR(ROTOR1) STR(ROTOR2) STR(ROTOR3) STR(ROTOR4) STR(ROTOR5) STR(ROTOR6) STR(ROTOR7) STR(ROTOR8)
               STR(UKWA) STR(UKWB) STR(UKWC) STR(ROTORB) STR(ROTORG) STR(UKWBD) STR(UKWCD)
               STR(ETWD) STR(ROTORD1) STR(ROTORD2) STR(ROTORD3) STR(UKWD)
               STR(ETWR) STR(ROTORR1) STR(ROTORR2) STR(ROTORR3) STR(UKWR)
               STR(ETWS) STR(ROTORS1) STR(ROTORS2) STR(ROTORS3) STR(UKWS));

   LOGOX1 = F("\x00\x00\x03\xFF\xFC\x00\x00\x00"
              "\x00\x00\xFF\xFF\xFF\xF0\x00\x00"
              "\x00\x07\xFC\x00\x03\xFE\x00\x00"
              "\x00\x3F\x00\x00\x00\x0F\xC0\x00"
              "\x00\xF8\x01\x8F\x1F\x01\xF0\x00"
              "\x03\xC0\xF9\x9F\x9F\xF0\x3C\x00"
              "\x07\x03\xF9\x99\x99\xF8\x0E\x00"
              "\x0C\x33\x19\x98\x19\x99\x83\x00"
              "\x19\xF3\x19\x98\x19\x99\xE1\x80"
              "\x39\x83\x19\x98\x19\x99\xF9\xC0"
              "\x79\x83\x19\x98\x19\x99\x99\xE0"
              "\xDD\xE3\x19\x9B\x99\x99\x9B\xB0"
              "\x8D\xE3\x19\x9B\x99\x99\xFB\x10"
              "\xDD\x83\x19\x99\x99\x99\xFB\xB0"
              "\x79\x83\x19\x99\x99\x99\x99\xE0"
              "\x39\xC3\x19\x99\x99\x99\x99\xC0"
              "\x19\xF3\x19\x99\x99\x99\x91\x80"
              "\x0C\x33\x19\x99\x99\x99\x83\x00"
              "\x07\x03\x19\x99\x99\x99\x0E\x00"
              "\x03\xC0\x19\x9F\x99\x90\x3C\x00"
              "\x00\xF8\x19\x8F\x19\x01\xF0\x00"
              "\x00\x3F\x00\x00\x00\x0F\xC0\x00"
              "\x00\x07\xFC\x00\x03\xFE\x00\x00"
              "\x00\x00\xFF\xFF\xFF\xF0\x00\x00"
              "\x00\x00\x03\xFF\xFC\x00\x00\x00");

   LOGOX2 = F("\x00\x00\x00\x00\x03\xFF\xFF\xFF\xFF\xFC\x00\x00\x00\x00\x00"
              "\x00\x00\x00\x00\x3F\xFF\xFF\xFF\xFF\xFF\xC0\x00\x00\x00\x00"
              "\x00\x00\x00\x07\xFF\xFF\xFF\xFF\xFF\xFF\xFE\x00\x00\x00\x00"
              "\x00\x00\x00\x3F\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xC0\x00\x00\x00"
              "\x00\x00\x01\xFF\xFF\xF0\x00\x00\x00\xFF\xFF\xF8\x00\x00\x00"
              "\x00\x00\x0F\xFF\xFE\x00\x00\x00\x00\x07\xFF\xFF\x00\x00\x00"
              "\x00\x00\x3F\xFF\x80\x00\x00\x00\x00\x00\x1F\xFF\xC0\x00\x00"
              "\x00\x00\xFF\xFC\x00\x00\x00\x00\x00\x00\x03\xFF\xF0\x00\x00"
              "\x00\x03\xFF\xE0\x00\x0F\x03\xFC\x0F\xFF\xC0\x7F\xFC\x00\x00"
              "\x00\x0F\xFF\x00\x00\x0F\x07\xFE\x0F\xFF\xF0\x0F\xFF\x00\x00"
              "\x00\x3F\xF8\x03\xFF\x0F\x0F\xFF\x0F\xFF\xFC\x01\xFF\xC0\x00"
              "\x00\x7F\xC0\x0F\xFF\x0F\x0F\xFF\x0F\xFF\xFE\x00\x3F\xE0\x00"
              "\x00\xFF\x00\x3F\xFF\x0F\x0F\x0F\x0F\x0F\x1F\x00\x0F\xF0\x00"
              "\x01\xFC\x00\x3F\xFF\x0F\x0F\x0F\x0F\x0F\x0F\x00\x03\xF8\x00"
              "\x03\xF0\x3C\x3C\x0F\x0F\x0F\x00\x0F\x0F\x0F\x0F\x00\xFC\x00"
              "\x07\xC1\xFC\x3C\x0F\x0F\x0F\x00\x0F\x0F\x0F\x0F\xC0\x3E\x00"
              "\x0F\x87\xFC\x3C\x0F\x0F\x0F\x00\x0F\x0F\x0F\x0F\xF0\x1F\x00"
              "\x1F\x0F\xE0\x3C\x0F\x0F\x0F\x00\x0F\x0F\x0F\x0F\xFC\x0F\x80"
              "\x3F\x0F\x00\x3C\x0F\x0F\x0F\x00\x0F\x0F\x0F\x0F\xFF\x0F\xC0"
              "\x3F\x0F\x00\x3C\x0F\x0F\x0F\x00\x0F\x0F\x0F\x0F\xFF\x0F\xC0"
              "\x7F\x0F\x00\x3C\x0F\x0F\x0F\x00\x0F\x0F\x0F\x0F\x0F\x0F\xE0"
              "\x7F\x0F\x00\x3C\x0F\x0F\x0F\x00\x0F\x0F\x0F\x0F\x0F\x0F\xE0"
              "\xFF\x8F\xF0\x3C\x0F\x0F\x0F\x3F\x0F\x0F\x0F\x0F\x0F\x1F\xF0"
              "\xF3\xCF\xF0\x3C\x0F\x0F\x0F\x3F\x0F\x0F\x0F\x0F\x0F\x3C\xF0"
              "\xE1\xCF\xF0\x3C\x0F\x0F\x0F\x3F\x0F\x0F\x0F\x0F\xFF\x38\x70"
              "\xE1\xCF\xF0\x3C\x0F\x0F\x0F\x3F\x0F\x0F\x0F\x0F\xFF\x38\x70"
              "\xF3\xCF\x00\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\xFF\x3C\xF0"
              "\xFF\x8F\x00\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\xFF\x1F\xF0"
              "\x7F\x0F\x00\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\xE0"
              "\x7F\x0F\x00\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\xE0"
              "\x3F\x0F\xC0\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\xC0"
              "\x3F\x0F\xC0\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\xC0"
              "\x1F\x0F\xE0\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0E\x0F\x80"
              "\x0F\x87\xFC\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0C\x1F\x00"
              "\x07\xC1\xFC\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x00\x3E\x00"
              "\x03\xF0\x3C\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x00\xFC\x00"
              "\x01\xFC\x00\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0E\x03\xF8\x00"
              "\x00\xFF\x00\x3C\x0F\x0F\x0F\x0F\x0F\x0F\x0F\x0C\x0F\xF0\x00"
              "\x00\x7F\xC0\x00\x0F\x0F\x0F\xFF\x0F\x0F\x0E\x00\x3F\xE0\x00"
              "\x00\x3F\xF8\x00\x0F\x0F\x0F\xFF\x0F\x0F\x0C\x01\xFF\xC0\x00"
              "\x00\x0F\xFF\x00\x0F\x0F\x07\xFE\x0F\x0E\x00\x0F\xFF\x00\x00"
              "\x00\x03\xFF\xE0\x0F\x0F\x03\xFC\x0F\x0C\x00\x7F\xFC\x00\x00"
              "\x00\x00\xFF\xFC\x00\x00\x00\x00\x00\x00\x03\xFF\xF0\x00\x00"
              "\x00\x00\x3F\xFF\x80\x00\x00\x00\x00\x00\x1F\xFF\xC0\x00\x00"
              "\x00\x00\x0F\xFF\xFE\x00\x00\x00\x00\x07\xFF\xFF\x00\x00\x00"
              "\x00\x00\x01\xFF\xFF\xF0\x00\x00\x00\xFF\xFF\xF8\x00\x00\x00"
              "\x00\x00\x00\x3F\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xC0\x00\x00\x00"
              "\x00\x00\x00\x07\xFF\xFF\xFF\xFF\xFF\xFF\xFE\x00\x00\x00\x00"
              "\x00\x00\x00\x00\x3F\xFF\xFF\xFF\xFF\xFF\xC0\x00\x00\x00\x00"
              "\x00\x00\x00\x00\x03\xFF\xFF\xFF\xFF\xFC\x00\x00\x00\x00\x00");

   enigma_state = SPLASH_STATE;

   draw_display();
}  // setup()


// show initialization over serial
void show_init_over_serial(void)
{
   Serial.println();
   Serial.print(" Enigma ");
   Serial.println(TITLE);
   Serial.print(VERSION0);
   Serial.println(VERSION1);
   Serial.println();
   Serial.println(TAPSTART);
   Serial.println();
   Serial.println();
}  // show_init_over_serial()


// show the current machine settings over serial
void show_machine_over_serial(void)
{
   Serial.println("");
   Serial.println("");
   switch(EnigmaCurrentData.machine_type)
   {
      case MACHINE_TYPE_M3:
      {
         Serial.println(F("[ Enigma M3 (1939) ]"));
      }
      break;

      case MACHINE_TYPE_M4:
      {
         Serial.println(F("[ Enigma M4 (1942) ]"));
      }
      break;

      case MACHINE_TYPE_ENIGMA_D:
      {
         Serial.println(F("[ Business Enigma D ]"));
      }
      break;

      case MACHINE_TYPE_ROCKET_K:
      {
         Serial.println(F("[ Rocket K Railway ]"));
      }
      break;

      case MACHINE_TYPE_SWISS_K:
      {
         Serial.println(F("[ Swiss K ]"));
      }
      break;
   }

   Serial.println("");
   Serial.print(F("Stecker                 : "));
   Serial.println(EffSTECKER);
}  // show_machine_over_serial()


// send the current value of the rotors out serial
void show_rotors_over_serial(void)
{
   const char *charptr PROGMEM = (const char PROGMEM *)WHEELSF;
   char k;
   unsigned int wheeltype;

   for (byte i = 0; i < 6; i++)
   {
      if (EnigmaCurrentData.WHEELTYPE[i] != 0)
      {
         switch (i)
         {
            case 0:
            {
               Serial.print(F("ETW (input)             : "));
            }
            break;

            case 1:
            {
               Serial.print(F("R1  (rightmost)         : "));
            }
            break;

            case 2:
            {
               Serial.print(F("R2  (2nd from right)    : "));
            }
            break;

            case 3:
            {
               Serial.print(F("R3  (leftmost of three) : "));
            }
            break;

            case 4:
            {
               Serial.print(F("R4  (leftmost of four)  : "));
            }
            break;

            case 5:
            {
               Serial.print(F("UKW (reflector)         : "));
            }
            break;
         }

         wheeltype = ((EnigmaCurrentData.WHEELTYPE[i] - 1) * 28) + 2;

         for (byte i = 0; i < 26; i++)
         {
            k = pgm_read_byte(charptr + wheeltype + i);
            Serial.print(k);
         }
         Serial.println("");
     }
   }
}  // show_rotors_over_serial()


//
// Sample Enigma messages for testing/verification ( from http://franklinheath.co.uk )
//
//
// =======================================
//              Example 1930
// =======================================
// Machine   : M3
// Reflector : A
// Wheels    : II I III
// Rings     : 24 13 22
// Plugs     : AM FI NV PS TU WZ
//
// Key       : ABL
//
// Encrypted:
// gcdse ahugw tqgrk vlfgx ucalx vymig
// mmnmf dxtgn vhvrm mevou yfzsl rhdrr
// xfjwc fhuhm unzef rdisi kbgpm yvxuz
//
// Decrypted:
// feind liqei nfant eriek olonn ebeob
// aqtet xanfa ngsue dausg angba erwal
// dexen dedre ikmos twaer tsneu stadt
//
// German:
// Feindliche Infanterie Kolonne beobachtet.  Anfang Sudausgang Barwalde. Ende 3km ostwarts Neustadt.
//
// English:
// Enemy infantry column was observed. Beginning [at] southern exit [of] Baerwald. Ending 3km east of Neustadt.
//
// =======================================
//              Turing 1940
// =======================================
// Machine   : Rocket K Railway
// Reflector : N/A
// Wheels    :    III I II
// Rings     : 26 17 16 13
// Plugs     : N/A
//
// Key       : JEZA
//
// Encrypted:
// qszvi dvmpn exacm rwwxu iyoty ngvvx
// dz
//
// Decrypted:
// deuts qetru ppens indje tztin engla
// nd
//
// German:
// Deutsche Truppen sind jetzt in England.
//
// English:
// German troops are now in England.
//
//
// =======================================
//             Barbarosa 1941
// =======================================
// Machine   : M3
// Reflector : B
// Wheels    : II IV V
// Rings     : 02 21 12
// Plugs     : AV BS CG DL FU HZ IN KM OW RX
//
// Key       : BLA
//
// Encrypted:
// edpud nrgys zrcxn uytpo mrmbo fktbz
// rezkm lxlve fguey siozv eqmik ubpmm
// ylklt tdeis mdica gykua ctcdo mohwx
// muuia ubsts lrnbz szwnr fxwfy ssxjz
// vijhi dishp rklka yupad txqsp inqma
// tlpif svkda sctac dpbop vhjk
//
// Decrypted:
// aufkl xabte ilung xvonx kurti nowax
// kurti nowax nordw estlx sebez xsebe
// zxuaf flieg erstr aszer iqtun gxdub
// rowki xdubr owkix opots chkax opots
// chkax umxei nsaqt drein ullxu hrang
// etret enxan griff xinfx rgtx
//
// German:
// Aufklarung abteilung von Kurtinowa Kurtinowa nordwestlich Sebez Sebez [auf]
// Fliegerstase in Richtung Dubrowki Dubrowki, Opotschka Opotschka. Um 18:30 Uhr
// angetreten angriff. Infanterie Regiment
//
// English:
// Reconnaisance division from Kurtinowa north-west of Sebech on the flight
// corridor towards Dubrowki, Opochka. Attack begun at 18:30 hours. Infantry
// Regiment
//
// Key       : LSD
//
// Encrypted:
// sfbwd njuse gqobh krtar eezmw kpprb
// xohdr oeqgb bgtqv pgvkb vvgbi mhusz
// ydajq iroax sssnr ehygg rpise zbovm
// qiemm zcysg qdgre rvbil ekxyq irgir
// qnrdn vrxcy ytnjr
//
// Decrypted:
// dreig ehtla ngsam abers iqerv orwae
// rtsxe inssi ebenn ullse qsxuh rxroe
// mxein sxinf rgtxd reixa uffli egers
// trasz emita nfang xeins seqsx kmxkm
// xostw xkame necxk 
//
// German:
// 3 geht langsam aber sicher vorwarts. 17:06 Uhr rom eins Infanterie Regiment 3
// auf Fliegerstase mit Anfang 16km km ostwarts Kamanec K.
//
// English:
// 3 goes slowly but surely forwards.  17:06 hours [Roman Numeral I] Infantry
// Regiment 3 on the flight corridor starting 16km east of Kamanec.
//
//
// =======================================
//               U-264 1942
// =======================================
// Machine   : M4
// Reflector : Thin B
// Wheels    : beta II IV I
// Rings     : 01 01 01 22
// Plugs     : AT BL DF GJ HM NW OP QY RZ VX
//
// Key       : VJNA
//
// Encrypted:
// nczw vusx pnym inhz xmqx sfwx wlkj ahsh
// nmco ccak uqpm kcsm hkse inju sblk iosx
// ckub hmll xcsj usrr dvko hulx wccb gvli
// yxeo ahxr hkkf vdre wezl xoba fgyu jquk
// grtv ukam eurb veks uhhv oyha bcjw makl
// fklm yfvn rizr vvrt kofd anjm olbg ffle
// oprg tflv rhow opbe kvwm uqfm pwpa rmfh
// agkx iibg
//
// Decrypted:
// vonv onjl ooks jhff ttte inse insd reiz
// woyy qnns neun inha ltxx beia ngri ffun
// terw asse rged ruec ktyw abos xlet zter
// gegn erst andn ulac htdr einu luhr marq
// uant onjo tane unac htse yhsd reiy zwoz
// wonu lgra dyac htsm ysto ssen achx ekns
// vier mbfa ellt ynnn nnno oovi erys icht
// eins null
//
// German:
// Von Von 'Looks' F T 1132/19 Inhalt: Bei Angriff unter Wasser gedruckt,
// Wasserbomben. Letzter Gegnerstandort 08:30 Uhr Marine Quadrat AJ9863, 220 Grad,
// 8sm, stosse nach. 14mb fallt, NNO 4, Sicht 10.
//
// English:
// From Looks, radio-telegram 1132/19 contents: Forced to submerge under attack,
// depth charges. Last enemy location 08:30 hours, sea square AJ9863, following 220
// degrees, 8 knots. [Pressure] 14 millibars falling, [wind] north-north-east 4,
// visibility 10.
//
//
// =======================================
//            Scharnhorst 1943
// =======================================
// Machine   : M3
// Reflector : B
// Wheels    : III VI VIII
// Rings     : 01 08 13
// Plugs     : AN EZ HK IJ LR MQ OT PV SW UX
//
// Key       : UZV
//
// Encrypted:
// ykae nzap msch zbfo cuvm rmdp ycof hadz
// izme fxth flol pzlf ggbo tgox gret dwtj
// iqhl mxvj wkzu astr
//
// Decrypted:
// steue rejta nafjo rdjan stand ortqu
// aaacc cvier neunn eunzw ofahr tzwon
// ulsmx xscha rnhor sthco
//
// German:
// Steuere Tanafjord an. Standort Quadrat AC4992, fahrt 20sm. Scharnhorst. [hco -
// padding?]
//
// English:
// Heading for Tanafjord. Position is square AC4992, speed 20 knots. Scharnhorst.
//
// End of sample Enigma messages for testing/verification
//
