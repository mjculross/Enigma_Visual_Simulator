# Enigma_Visual_Simulator
Graphical simulation of the Enigma Machine which runs on an Arduino+Adafruit touchscreen

Enigma Visual version 1.3 dated 03/10/2019 @2110
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
      over the serial port.

CONTACT INFO:

   If you have any questions, suggestions for improvement, and most especially
      any bug reports, please contact me at kd5rxt@arrl.net.

OPERATION:

   See the "Operation.txt" file for detailed operating instructions.

VERIFICATION:

   See the "Enigma_Sample_Messages.txt" file for sample Enigma messages which
   can be used to verify correct operation of the Enigma Visual Simulator.
