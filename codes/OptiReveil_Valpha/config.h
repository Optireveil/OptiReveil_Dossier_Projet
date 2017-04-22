/*
 * This is the configuration file of the code.
 * You will find the pins of all components linked to the Arduino
 * And you will find some constant used in the code.
 */

//###### PINS ######

// button pins
#define b1 4
#define b2 5
#define b3 6
#define b4 7

#define jack 10 //Jack pin (by default, the light
#define usb 11 //USB pin (by default, the captor)
#define ser 2 //SerLCD module pin
#define speaker 9 //Speaker pin

//###### CONSTANTS ######

#define blinking_hour_delay 1000 // time in milliseconds for the bliking cycle
#define long_press_time 700 // time in millisecond. We consider that if you press a button longer than long_press_time, your press is a long_press. else, this is a short press
#define quit_mode 15 //time in second. We considere that if we are in the configuring alarm or hour mode and we exceed this value, we return to the main mode
#define time_starting_night 20 //time in minute
