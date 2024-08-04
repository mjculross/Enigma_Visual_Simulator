# Enigma_Visual_Simulator
Graphical simulation of the Enigma Machine which runs on an Arduino+Adafruit touchscreen

Enigma Visual version 1.4 dated 07/23/2024 @1045
   written by Mark J Culross, KD5RXT (kd5rxt@arrl.net)

HARDWARE:

   This project runs on an Arduino Uno or equivalent.  For me personally, I
      prefer the SparkFun RedBoard - Programmed with Arduino (DEV-13975) -
      https://www.sparkfun.com/products/13975

   For the visual (graphical) version of the Enigma machine, this project uses
      the Adafruit 2.8" TFT Touch Shield for Arduino with Resistive Touch
      Screen (1651) - https://www.adafruit.com/product/1651

   This particular display provides touchscreen input & 320x240 full color
      display using the Adafruit Touch Shield V2 library.

DISPLAY CALIBRATION:

   The coordinates returned from the touchscreen will vary slightly from unit
      to unit, so if you find that the point where you touch on the screen is
      not matching the point where the Enigma Visual Simulator program
      responds, then you should run the TFTcal-Adafruit.ino calibration sketch.
      Doing so will help to determine the touchpad scaling constants TS_MINX,
      TS_MINY, TS_MAXX, & TS_MAXY on-the-fly which will work best for your
      particular hardware.  NOTE: use something small but not too sharp
      (e.g. a toothpick) to poke the touchscreen during calibration.  ALSO
      NOTE: this sketch also now includes a #define STMPE610_TOUCH_CONTROLLER
      line in the source.  See the description for its proper use below in
      the TOUCH CONTROLLER TYPE section.

TOUCH CONTROLLER TYPE:

   There's now a #define STMPE610_TOUCH_CONTROLLER line in the source that,
      when uncommented, provides support for older versions of the Adafruit
      2.8" TFT Touch Shield for Arduino with Resistive Touch Screen (1651)
      that previously used the STMPE610 touch controller.  If this define is
      commented out (which it is by default), then support is provided for
      the TSC2007 touch controller now included on the newer versions of the
      same Adafruit product (1651).  NOTE that the TFTcal_Adafruit.ino file
      only supports calibration of the older STMPE610 touchscreen controller.
      
IMPLEMENTATION:

   The design of the encode/decode processing in this sketch is based upon the
      original encode/decode engine from the EnigmaSerial.ino sketch (w/ some
      cleanup, optimization, & enhancements, all w/ no apparent adverse effects
      on equivalent/correct operation)

   My primary goal for this project was simulate as many versions of the Enigma
      machine as I could, with the ability to verify correct operations for
      each.  All versions of the Enigma machine that are simulated were
      verified to operate correctly using the excellent/versatile HTML-code
      utility titled "Universal Enigma Simulator v2.0" by Daniel Palloks,
      available at the following URL:
      http://people.physik.hu-berlin.de/~palloks/js/enigma/index_en.html

   Verification of correct operations for each of the machines was facilitated
      by using the sample Enigma settings & messages available at the following
      URL: http://franklinheath.co.uk

COMPILER/LIBRARY NOTE:

   If this version is built using older versions of the IDE and/or some
      versions of the Adafruit TFT library, the resulting compiled code could
      exceed the available program space of an Arduino Uno.  When this version
      is built with IDE version 1.8.7, the resulting code is efficient enough
      that it fits within the available program space.  If you find that the
      build process reports that the available program space is exceeded, then
      simply comment out the line "#define SERIAL_MODE", essentially removing
      the calls for run-time serial input & output, which should thus avoid
      exceeding the available program space until the IDE and/or Adafruit TFT
      library can be updated to newer versions.  Of course, doing so also
      results in the loss of the otherwise available serial capabiltity for
      processing Enigma messages & the ability to display the intricate
      details of the internal mechanical operations of the Enigma machine
      over the serial port.  NOTE: this define is now commented out by
      default, as the bloated libraries almost always result in exceeding
      available program space.  Sorry !!

CONTACT INFO:

   If you have any questions, suggestions for improvement, and most especially
      any bug reports, please contact me at kd5rxt@arrl.net.

OVERVIEW VIDEO:

   For a quick & dirty overview, see the following video:
     https://youtu.be/8nu2KaLVWrU

OPERATION:

   See the "Operation.txt" file for detailed operating instructions.

VERIFICATION:

   See the "Enigma_Sample_Messages.txt" file for sample Enigma messages which
   can be used to verify correct operation of the Enigma Visual Simulator.
