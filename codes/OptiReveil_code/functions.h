
void printHour(short hour2print, short min2print, boolean blinking=false){
  /*
   * This function prints the given hour on the LCD screen in a format hh:mm
   */
   serLCD com1 (ser);
   com1.setCursor(1,6); //put the cursor at the center (minus 2) of the first line
   if(hour2print < 10){
    com1.print("0"); // if the hours have just one digit, we add a 0 before the hour
   }
   com1.print(hour2print);
   com1.print(":");
   if(min2print < 10){
    com1.print("0"); // if the minutes have just one digit, we add a 0 before the hour
   }
  com1.print(min2print);
  if (blinking == true){
    if (millis()%blinking_hour_delay*2 >= blinking_hour_delay){ //you can configure blinking_hour_delay dans configure.h
      com1.setCursor(1,6);
      com1.print("  :  "); //erease the hour
    }
  }
}

short readButtons(){
  /*
   * This function reads the given buttons.
   * If one is pressed, it controls if it's a long or a short press
   * - returns 0 : if no button is pressed
   * - returns 1 : if b1 is short-pressed
   * - returns 2 : if b2 is short-pressed
   * - etc...
   * - returns (-1) : if b1 is long-pressed
   * - returns (-2) : if b2 is long-pressed
   * - etc...
   */
  serLCD com2 (ser);
  const short buttons2read[] = {b1,b2,b3,b4};
  unsigned long timeSave = 0; //used to detect a long press or a short press
  short int buttonState = 0; // used to record the state of a button
  bool long_press = false;

  for (int i=0; i<4; i++){
    buttonState = digitalRead(buttons2read[i]);
    if (buttonState == LOW){   //buttonState : LOW > pressed | HIGH > released
      timeSave = millis();

      while(buttonState == LOW){
        if(millis() - timeSave >= long_press_time){
          long_press = true;
          com2.setCursor(2,16);
          com2.print("^"); // print a symbol to show that the long press has been detected and the user can now release the button
        }
        buttonState = digitalRead(buttons2read[i]);
      }
      com2.setCursor(2,16);
      com2.print(" ");
      com2.setCursor(1,1);
      if(long_press == true){
         com2.print((-1)*(i+1));
        return((-1)*(i+1));
      } else {
        com2.print(i+1);
        return(i+1);
      }
    }
  }
  com2.print(0);
  return(0);
}

void updateHour (byte* hours_pointer, byte* minutes_pointer, byte* ref_hours_pointer, byte* ref_minutes_pointer){
  /*
   * Update the global hour in function of the internal clock of the Arduino.
   * notice that it takes pointers in parameters. so you have to call the function like that : UpdateHour( &hours, &minutes, &ref_hours, &ref_minutes)
   */

  short actual_time = ((millis()/1000)/60)%(60*24);       //takes the number of minutes since the arduino has started on 24hours : if the arduino is on since 24 hours, actual_time will get back to 0.
  *hours_pointer = actual_time/60;                                //takes the number of hours since the arduino has started
  *minutes_pointer = actual_time-(*hours_pointer*60); //takes the number of minutes since the arduino has started minus the number of hours
  if(*minutes_pointer + *ref_minutes_pointer >= 60){   // control if the number of minutes when we add ref_minutes don't exceed 60, in which case we add 1 to the hours
    *hours_pointer ++;
  }
  *minutes_pointer = (*ref_minutes_pointer+*minutes_pointer)%60; //takes the number of minutes modulo 60
  *hours_pointer = (*ref_hours_pointer+*hours_pointer)%24;           // takes the number of hours modulo 24
}

void setHour (byte* hours_pointer, byte* minutes_pointer, byte* ref_hours_pointer, byte* ref_minutes_pointer){
  /*
   * function that modify the ref_hours and ref_minutes variable in order to cofigure the hour
   */
   serLCD com (ser);
   com.clear();
   com.setCursor(2,1);
   com.print("SetHour");

   byte loc_hours = *hours_pointer, loc_minutes = *minutes_pointer;
   byte loc_ref_hours = *ref_hours_pointer, loc_ref_minutes = *ref_minutes_pointer;
   unsigned long quit_mode_time = millis()/1000; //takes the actual time in seconds

   while (millis()/1000 - quit_mode_time <= quit_mode){ //you can configure quit_mode in configure.h
     short l_pushed_buttons = readButtons();
     if(l_pushed_buttons != 0){
      quit_mode_time = millis()/1000; //If we press any button, we reset the exit chrono time
     }

     switch (l_pushed_buttons){
      case 0:
        updateHour(&loc_hours, &loc_minutes, &loc_ref_hours, &loc_ref_minutes);
        printHour(loc_hours, loc_minutes, true);
      break;

      case -1 :
        quit_mode_time -= quit_mode+1; //add time to quit the mode
      break;

      case 1:
        if(loc_ref_hours <= 0){
          loc_ref_hours = 23;
        } else {
          loc_ref_hours-- ;
        }
      break;

      case 2:
        if(loc_ref_hours >= 24){
          loc_ref_hours = 0;
        } else {
        loc_ref_hours++ ;
        }
      break;

      case 3:
        if(loc_ref_minutes <= 0){
          loc_ref_minutes = 59;
        } else {
        loc_ref_minutes-- ;
        }
      break;

      case 4:
        if(loc_ref_minutes >= 60){
          loc_ref_minutes = 0;
        } else {
        loc_ref_minutes++ ;
        }
      break;
     }
   }
   com.clear();
   *ref_hours_pointer = loc_ref_hours;
   *ref_minutes_pointer = loc_ref_minutes;
}
