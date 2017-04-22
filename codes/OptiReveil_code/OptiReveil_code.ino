#include <SoftwareSerial.h> // a native library that allow to use other pin than D0 and D1 to communicate in serial
#include <serLCD.h> // a SparkFun library which allow to use in an easiest way the SparkFun SerLCD module (you can download it there : https://github.com/yazug/serLCD )
#include <Adafruit_NeoPixel.h> // an adafruit library that allow to control 3 colors led modules "NeoPixel" (lib can be downloaded here : https://github.com/adafruit/Adafruit_NeoPixel )

#include "configure.h" //Inclusion of the pin configuration file
#include "functions.h"  // Inclusion of the different
#include "night_mode.h" // files of the code

boolean night_mode = false; // true : nightMode || false : dayMode

//### settings at begining ###
byte ref_hours = 23, ref_minutes = 59; //hours and minutes in more or in less compared to the internal arduino time (ref_hours goes from 0 to 23 and ref_minutes goes from 0 to 59)
byte hours = 0, minutes = 0; //hours and minutes displayed on the clock [hours = ((millis()/1000)/3600)+ref_hours | minutes = ((millis()/1000)/60)-((hours-ref_hours)*60) ]=> arduino truncs automaticly numbers which are not bytes
byte alm_hours = 0, almMinutes = 0; //hours and minutes of the alarm
byte alm_mode = 0; // |0: off |1: sound |2: sound+light
byte backlight_intens = 2; //intensity of the LCD screen backlight. Goes from 0 to 3

serLCD com(ser); // pin connected to the SerLCD module

void setup() {
  const byte buttons[] = {b1,b2,b3,b4}; // pin connected to buttons

  for (byte i=0; i<4; i++){
    pinMode(buttons[i], INPUT); //declaration of each button
    digitalWrite(buttons[i], HIGH); //acivating pull-up resistor, so that you just have to use 2 wires instead of 3 to connect your button
  }
  com.clear(); // making sure that there is nothing on the screen
  printHour(hours, minutes);

}

void loop() {
  short pushed_button = readButtons(); // read the buttons that are pressed
  switch (pushed_button){
    case 0: //nothing is pressed : update hour, and prompt the selected alarm mode
      updateHour(&hours, &minutes, &ref_hours, &ref_minutes);
      printHour(hours, minutes);
    break;

    case 1: //ambiant light on/off

    break;

    case -1: //configure hour
      setHour(hours, minutes, ref_hours, ref_minutes);
    break;

    case 2: //change alarm mode
    break;

    case -2: //configure alarm time
    break;

    case 3: //nothing (for the moment)
    break;

    case -3: //night mode
    break;

    case 4: //change backlight intensity
    break;

    case -4: //see the night review
    break;
  }
}
