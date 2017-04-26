#include <SoftwareSerial.h>
#include <serLCD.h>

#include "config.h"

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, jack, NEO_GRB + NEO_KHZ800);
// MARK: declarations
serLCD com(ser); //pin du module Ser lcd
int bout[] = {b1,b2,b3,b4}; //pins des boutons
int state = 1; //état d'un bouton => 0: appuyé; 1: relaché
int cligno = 100; //
boolean ctrlLong = false; //controle d'un appuie long
long unsigned int timeSave = 0; // Sauvegarde du temps
boolean control = true; //
int pushBout = 8; //Bouton appuyé : 0~3 : court; 4~7 : long; 8 : aucun
long unsigned int takeHour = millis(); //
int minute = 59; // Minutes du réveil
int heure = 23;  // Heures du réveil
int heureAlm = 0;
int minuteAlm = 0;
boolean modeNuit = false; // mode nuit activé ou non
int changeurLum = 0; //de 0 a 3, variable qui sait sur quel backlight on est
int modeAlm = 0; // 0:off 1:son 2:lum+son
boolean alm_chk = true;

int print_night_mode = false;

int light_blue = 0;
int light_red = 0;
int light_green = 0;
boolean ambiant_light_mode = false;
unsigned long int chrono_aube = 0;
int target_red = 0;
int target_green = 0;
int target_blue = 0;
int old_tic = 0;
int total_time = 1;

//### NIGHT MODE ###
boolean mode_nuit = false;
boolean wait_sleep = false;
boolean first_time_track = true;
boolean track_start = false;
boolean first_time_insert_table = true;

unsigned long int chrono_wait_for_sleep = 0;
unsigned long int chrono_track_start = 0;
byte check_table_size[144]; //(max_night_hours*60/time_record_moves)
int time_code_night[144];
int night[144];
int count_night = 0;
int count_five_min = 0;
int etalonnage_capt = captor_calibration;
int seuil_detect_mouv = ceil_detection;






void setup(){
  strip.begin();
  strip.show();
  setupSpecialChar();
  for (int i=0; i<4; i++){
    pinMode(bout[i], INPUT); //déclaration des entrées Boutons
    digitalWrite(bout[i], HIGH); //activation des résistances Pull-up
  }
  pinMode(usb, INPUT);
  pinMode(speaker, OUTPUT);
  com.clear();
  afficherHeure(heure, minute);
}

void loop(){
  // MARK: day mode
  checkAlarm();
  pushBout = readBut();
    switch (pushBout){
      case 0:
        //Ambiant light on/off
        endNight();
        if (ambiant_light_mode == false){
          chrono_aube = millis();
          ambiant_light_mode = true;
          light_red = 0;
          light_green = 0;
          light_blue = 0;
        } else {
          ambiant_light_mode = false;
        }
      break;

      case 1:
        endNight();
        changeModeAlarme();
        afficherAlarmMode();

      break;

      case 2:
        endNight();
        setupSpecialChar();
      break;

      case 3:
      endNight();
        changeLum();
      break;

      case 4:
      endNight();
        clignoHeure();
      break;

      case 5:
      endNight();
        clignoAlarme();
      break;

      case 6:

      if (mode_nuit == true){
        endNight();
      } else {
        mode_nuit = true;
        com.setCursor(1,16);
        com.printCustomChar(4);
      }
      break;

      case 7:
        //compte rendu de la nuit
        if (mode_nuit == false){
          print_night_mode = true;
          printNight();
        }else{
          endNight();
        }
      break;

      case 8:
        // debut actualisation

        actualiserHeure();
        afficherHeure(heure,minute);
        afficherAlarmMode();
        ambiance_light();
        if(mode_nuit == true){
          ambiant_light_mode = false;
          com.setCursor(1,16);
          com.printCustomChar(4);
          waitForSleep();
          monitoringNight();
        }else{
          com.setCursor(2,6);
          com.print("   ");
        }
        // fin actualisation de l'heure
      break;
  }
}

// fonction qui regarde l'etat des boutons, et détecte un appui long ou court
// MARK: lire boutons
int readBut(){
  for (int i=0; i<4; i++){
    state = digitalRead(bout[i]);
    if (state == 0){
      timeSave = millis();
      while (state == 0){
        if(millis() - timeSave >= long_press_time){
        //Voyant
        com.setCursor(2,16);
        com.printCustomChar(3);
        ctrlLong = true;
        }
        state = digitalRead(bout[i]);
      }
      if (ctrlLong == true){
        com.setCursor(2,16);
        com.print(" ");
        ctrlLong = false;
        return(i+4);
        break;
      }  else {
        return(i);
      }
    }
  }
    return(8);
}


// fonction pour faire clignoter l'heure et la modifier(à faire)
// MARK: config heure
void clignoHeure(){
  com.clear();
  unsigned long int timeCligno = millis(), newTimeCligno = millis();
  com.setCursor(2,1);
  com.print("Cfg Heure");
  while(newTimeCligno - timeCligno <= quit_mode*1000){
    newTimeCligno = millis();
    pushBout = readBut();
    if(pushBout != 8){
      timeCligno =millis();
      newTimeCligno = millis();
      afficherHeure(heure, minute);
    }
    switch (pushBout){
      case 8 :
        if((newTimeCligno-timeCligno)%blinking_hour_delay*2 <= blinking_hour_delay){
          afficherHeure(heure, minute);
        } else {
        cacherHeure();
        }
      break;
      case 1:
        heure++;
        if(heure>=24){
          heure=0;
        }
        afficherHeure(heure, minute);
      break;
      case 0:
        heure--;
        if(heure<0){
          heure=23;
        }
        afficherHeure(heure, minute);
      break;
      case 3:
        minute++;
        if(minute>59){
          minute=0;
        }
        afficherHeure(heure, minute);
      break;
      case 2:
        minute--;
        if(minute<0){
          minute=59;
        }
        afficherHeure(heure, minute);
      break;
      case 4:
        newTimeCligno = timeCligno + 15001;
      break;
    }
  }
  com.setCursor(2,1);
  com.print("         ");
}


// MARK: config alarme
void clignoAlarme(){
  com.clear();
  unsigned long int timeCligno = millis(), newTimeCligno = millis();
  com.setCursor(2,1);
  com.print("Cfg Alarm");
  while(newTimeCligno - timeCligno <= quit_mode*1000){
    newTimeCligno = millis();
    pushBout = readBut();
    if(pushBout != 8){
      timeCligno =millis();
      newTimeCligno = millis();
      afficherHeure(heureAlm, minuteAlm);
    }
    switch (pushBout){
      case 8 :
        if((newTimeCligno-timeCligno)%blinking_hour_delay*2 <= blinking_hour_delay){
          afficherHeure(heureAlm, minuteAlm);
        } else {
        cacherHeure();
        }
      break;
      case 1:
        heureAlm++;
        if(heureAlm>=24){
          heureAlm=0;
        }
        afficherHeure(heureAlm, minuteAlm);
      break;
      case 0:
        heureAlm--;
        if(heureAlm<0){
          heureAlm=23;
        }
        afficherHeure(heureAlm, minuteAlm);
      break;
      case 3:
        minuteAlm++;
        if(minuteAlm>59){
          minuteAlm=0;
        }
        afficherHeure(heureAlm, minuteAlm);
      break;
      case 2:
        minuteAlm--;
        if(minuteAlm<0){
          minuteAlm=59;
        }
        afficherHeure(heureAlm, minuteAlm);
      break;
      case 5:
        newTimeCligno = timeCligno + 15001;
      break;
    }
  }
  com.setCursor(2,1);
  com.print("         ");
}

// MARK: actualise l'heure
void actualiserHeure (){
  if (millis() - takeHour*1000 >= 60000){
    minute ++;
    takeHour += 60;
  }
  if (minute >= 60){
    heure ++;
    minute -=60;
  }
  if (heure >= 24){
    heure -= 24;
  }
}

// MARK: afficher heure
void afficherHeure(int heures, int minutes){
  com.setCursor(1,6);
  if (heures < 10){
    com.print("0");
  }
  com.print(heures);
  com.print(":");
  if (minutes < 10){
    com.print("0");
  }
  com.print(minutes);
}

// MARK: cacher heure
void cacherHeure(){
  com.setCursor(1,6);
  com.print("  :  ");
}

// MARK: modifier backlight
void changeLum(){
  if(changeurLum == 0){
    com.setBrightness(0);
    changeurLum ++;
  }
  else if(changeurLum == 1){
    com.setBrightness(10);
    changeurLum ++;
  }
  else if(changeurLum == 2){
    com.setBrightness(20);
    changeurLum ++;
  }
  else if(changeurLum == 3){
    com.setBrightness(29);
    changeurLum = 0;
  }

}

// MARK: changer mode alrme
void changeModeAlarme(){

  if(modeAlm == 0){
    modeAlm ++;

  }
  else if(modeAlm == 1){
    modeAlm ++;

  }
  else if(modeAlm == 2){
    modeAlm =0;
  }
}

// MARK: afficher mode alrme
void afficherAlarmMode(){
  com.setCursor(1,1);
  com.print("Alm  ");
  if(modeAlm == 1 ){
    com.setCursor(2,1);
    com.printCustomChar(1);
    com.print(" X");
  }
  else if(modeAlm == 2){
    com.setCursor(2,1);
    com.printCustomChar(1);
    com.print(" ");
    com.printCustomChar(2);
  }
  else{
    com.setCursor(2,1);
    com.print("X X");
  }

}

// MARK: end the night
void endNight (){
  com.setCursor(1,16);
  com.print(" ");
  com.setCursor(2,6);
  com.print("   ");
  chrono_wait_for_sleep = 0;
  chrono_track_start = 0;
  count_night = 0;
  count_five_min = 0;
  mode_nuit = false;
  wait_sleep = false;
  first_time_track = true;
  track_start = false;
  first_time_insert_table = true;
  mode_nuit = false;
}

// MARK: déclencher alarme
void startAlarm(){
  boolean stop_alarme = false;
  int note = 110;
  short direction = 1;
  while (stop_alarme == false){
    digitalWrite(speaker, HIGH);
    delayMicroseconds((1000000/note));
    digitalWrite(speaker, LOW);
    delayMicroseconds((1000000/note));
    if(note<=110){
      direction = 1;
    }else if(note >= 880){
      direction = -1;
    }
    note += direction;
    int pushed_button = readBut();
    if (pushed_button != 8){
      stop_alarme = true;
    }
  }
}

// MARK: vérifier si il faut déclencher alarme
void checkAlarm(){
  if(alm_chk == true){
    if(modeAlm != 0){
      if(heureAlm == heure && minuteAlm == minute){
        alm_chk = false;
        endNight();
        startAlarm();
      }
    }
  } else {
    if (heureAlm != heure || minuteAlm != minute){
      alm_chk = true;
    }
  }
}

// MARK: echantilloner
int echantilloner(boolean test_taux = false){
  int T = 1000/etalonnage_capt;
  int nb_detect = 0;
  int detect = 0;
  int taux = 0;

  for (int i=0; i<etalonnage_capt; i++){
    detect = digitalRead(usb);
    if (detect == 1){
      nb_detect ++;
    }
    delay(T);
  }
  taux = map(nb_detect, 0, etalonnage_capt, 0, 100);
  if (test_taux == true){
    if(taux >= seuil_detect_mouv){
      com.setCursor(2,7);
      com.printCustomChar(7);
      return(1);
    } else {
      com.setCursor(2,7);
      com.print(" ");
      return(0);
    }
  }else{
      return(taux);
  }
}


// MARK: attendre l'endormissement
void waitForSleep(){

  if(wait_sleep == false){
    wait_sleep = true;
    chrono_wait_for_sleep = millis()/(1000);
  }else if(track_start == false){
    int echantillon = echantilloner();
    if (echantillon >= seuil_detect_mouv){
      chrono_wait_for_sleep = millis()/(1000);
    }else{
      com.setCursor(2,6);
      com.printCustomChar(5);
    }
    if ((millis()/1000) - chrono_wait_for_sleep >= time_starting_night*60) {
      //.print("No mouv has been detected since ");
      //.println(millis()/(1000*60) - chrono_wait_for_sleep);
      track_start = true;
      com.setCursor(2,6);
      com.print(" ");
    }
  }
}

// MARK: moniteur de la nuit
void monitoringNight(){
  if(track_start == true){
    if(first_time_track == true){
      first_time_track = false;
      chrono_track_start = millis();
      count_night = 0;
      first_time_insert_table = true;
      check_table_size[0]=0;
      check_table_size[1]=0;
    }
    if(readBut() == 8){
      com.setCursor(2,8);
      com.printCustomChar(6);
      if((millis() - chrono_track_start)%30000 < 3000){
        com.setCursor(2,10);
        com.print("R");
        //Serial.println("Fourchette");
        if (first_time_insert_table == true){
          //Serial.println("Première fois Fourchette");
          first_time_insert_table = false;
          //Serial.print("count five : ");
          //Serial.println(count_five_min);
          night[count_night] = count_five_min;
          //Serial.print("night : ");
          //Serial.println(night[count_night]);
          time_code_night[count_night]=heure*100+minute;
          //Serial.print("time code : ");
          //Serial.println(time_code_night[count_night]);
          check_table_size[count_night] = 1;
          //Serial.print("ctrl table size : ");
          //Serial.print(check_table_size[count_night]);
          count_night += 1;
          check_table_size[count_night] = 0;
          //Serial.println(check_table_size[count_night]);
          count_five_min = 0;
        }
      } else {
        com.setCursor(2,10);
        com.print(" ");
        first_time_insert_table = true;
        //Serial.print("Detection en cours... ");
        check_table_size[count_night] = 0;
        count_five_min += echantilloner(true);
        //Serial.println(count_five_min);
      }
    }
  }

  /*if(track_start == false){
    chrono_track_start = millis();
    count_five_min = 0;
    count_night = 0;
    insert_in_table = true;
    insertion = true;
    check_table_size[1]=0;
  }
  if(track_start == true){
    int test_bout = readBut();
    if (test_bout != 8){
      mode_nuit = false;
      break;
    }
    com.setCursor(2,8);
    com.printCustomChar(6);
    if ((millis() - chrono_track_start)%(5*60*1000) <= 1000){
      insert_in_table = false;
      if (insertion == false){
        insertion = true;
        night[count_night] = count_five_min;
        check_table_size[count_night]=1;
        check_table_size[count_night+1]=0;
        int time_code = heure*100+minute;
        time_code_night[count_night] = time_code;
        count_five_min = 0;
        count_night++;
      }
    } else {
      int echantillon = echantilloner();
      if (echantillon >= seuil_detect_mouv){
        count_five_min++;
      }
      insertion = false;

    }
  }*/
}

void printNight (){
  com.clear();
  int index = 0;
  while(print_night_mode == true){
    int pressed_button = readBut();
    switch (pressed_button){
      case 0:
      if (index > 0){
        for(int a=0; night[index]/pow(10,a) >= 1; a++){
          com.print(" ");
        }
        com.print(" ");
        index --;
      }
      break;

      case 1:
      if (check_table_size[index+1]>=1){
        for(int a=0; night[index]/pow(10,a) >= 1; a++){
          com.print(" ");
        }
        com.print(" ");
        index ++;
      }
      break;

      case 7:
      print_night_mode = false;
      com.clear();
      break;

      case 8:
      com.setCursor(1,1);
      com.print("Tmp:");
      com.setCursor(2,1);
      com.print("Mov:");
      int heure_time_code = time_code_night[index]/100;
      int minute_time_code = time_code_night[index]-(heure_time_code*100);
      afficherHeure(heure_time_code, minute_time_code);
      com.setCursor(2,6);
      com.print(night[index]);
      break;
    }
  }
}

void setupSpecialChar(){
  //### SPECIAL CHARACTERS ###
  uint8_t sound_char[] = {
  B00000,
  B00010,
  B00110,
  B11010,
  B11010,
  B00110,
  B00010,
  B00000,
  };

  uint8_t light_char[] = {
  B00000,
  B10101,
  B01110,
  B11011,
  B01110,
  B10101,
  B00000,
  B00000,
  };

  uint8_t long_press_char[] = {
  B00100,
  B01110,
  B11111,
  B01110,
  B01110,
  B00000,
  B01110,
  B00000,
  };

  uint8_t night_mode_char[] = {
  B00000,
  B01110,
  B11101,
  B11000,
  B11101,
  B01110,
  B00000,
  B00000,
  };

  uint8_t wait_sleep_char[] = {
  B00000,
  B01110,
  B11101,
  B11011,
  B11111,
  B01110,
  B00000,
  B00000,
  };

  uint8_t tracking_char[] = {
  B00000,
  B10001,
  B01110,
  B01010,
  B01110,
  B11011,
  B00000,
  B00000,
  };

  uint8_t mouv_char[] = {
  B00000,
  B00010,
  B00100,
  B01110,
  B00100,
  B01000,
  B00000,
  B00000,
  };
  com.setCursor(2,6);
  com.createChar(1,sound_char);
  delay(5);
  com.printCustomChar(1);
  com.createChar(2,light_char);
  delay(5);
  com.printCustomChar(2);
  com.createChar(3,long_press_char);
  delay(5);
  com.printCustomChar(3);
  com.createChar(4,night_mode_char);
  delay(5);
  com.printCustomChar(4);
  com.createChar(5,wait_sleep_char);
  delay(5);
  com.printCustomChar(5);
  com.createChar(6,tracking_char);
  delay(5);
  com.printCustomChar(6);
  com.createChar(7,mouv_char);
  delay(5);
  com.printCustomChar(7);
  delay(500);
  com.setCursor(2,6);
  com.print("       ");
}

void ambiance_light(){
  if(ambiant_light_mode == true){
    int tic_unit = total_time*60000/573;
    int tic = ((millis()-chrono_aube)/tic_unit);
    if (tic <= 26){
      target_blue = 26;
      target_green = 0;
      target_red = 0;
    } else if (tic <= 64){
      target_blue = 15;
      target_green = 38;
      target_red = 0;
    } else if (tic <= 153){
      target_blue = 63;
      target_green = 63;
      target_red = 89;
    } else if (tic <= 318){
      target_blue = 102;
      target_green = 153;
      target_red = 165;
    } else {
      target_blue = 255;
      target_green = 250;
      target_red = 240;
    }
    if (old_tic != tic){
      old_tic = tic;
      if(light_red < target_red){
        light_red ++;
      }
      if(light_green < target_green){
        light_green ++;
      }
      if(light_blue < target_blue){
        light_blue ++;
      }
    }
      strip.setPixelColor(0,strip.Color(light_red, light_green, light_blue));
      strip.show();
    } else {
      strip.setPixelColor(0,strip.Color(0,0,0));
      strip.show();
    }
}
