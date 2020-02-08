//lol.
//You fucking clicked the link to read my code
//You fucking nerd.

/* Author: Kelsey Williams
 * Yeah, so, this is an alarm clock that
 * tases you instead of beeping.  I used
 * a TM1637 4 digit 7 seg display to display
 * time, a relay to control a ViperTech taser that 
 * I presoldered wires to to overide the manual button,
 * and a bunch of buttons on a bread board to set
 * alarm times and whatever.  Idfk.  It's 
 * absolutely retarded and thats what I love about
 * it. Anyways here's the pinout.
 * 
 *  ***pinout (as used by moi)***
 * TM1637 display digital i/o = 13
 * TM1637 display clock pin = 12 
 * button to switch between clock and alarm setup = 11
 * button to increment hours = 10
 * button to increment minutes = 9
 * button to start the alarm = 8
 * button to stop the alarm = 7
 * signal pin for relay = 6
 * pin for "armed" led = 5
 */
 /*Also. Fucking remember that the documentation for Button.h 
  * on arduino playground is utter horsehsit.  
  Don't even bother reading it.*/
#include <Button.h>
#include <TM1637Display.h>

// TM1637 Display's clock and io
uint16_t DIO = 13;
uint16_t CLK = 12;

// "Armed" led and relay signal pin
uint16_t ALED = 5;
uint16_t RELAY = 6;

/* This bool is used to switch the display from the current time
 *  (swtkey = true), to the set alarm time (swtkey = false).
 */
bool swtkey = true;

// The last pressed button
//P.S. -1 equates to null in this case as 0 is a valid character around these parts
int16_t key = -1;

Button swt = Button(11);
Button inch = Button(10);
Button incm = Button(9);
Button stt = Button(8);
Button stp = Button(7);

// Clock time
uint16_t minutes = 0;
uint16_t hours =  12;

// Alarm time
uint16_t aminutes = 0;
uint16_t ahours = 12;

// Alarm booleans
bool alarmOn = false;
bool alarmTripped = false;

// Main alarm clock time
uint32_t delta = millis();
uint32_t elapsedMilliseconds = 0;

// Time to measure pulses between zaps
uint32_t trippedTime;

uint16_t count = 0;

TM1637Display display = TM1637Display(CLK, DIO);

void setup() {
  swt.begin();
  inch.begin();
  incm.begin();
  stt.begin();
  stp.begin();
  
  pinMode(ALED, OUTPUT);
  pinMode(RELAY, OUTPUT);

  // Intialize the display
  display.clear();
  delay(1000);
  display.setBrightness(2);
}
void loop() {
  keyListener();
  // Count the time in milliseconds
  elapsedMilliseconds = millis() - delta;
  countTime();
  // Display the time
  if(swtkey){
    DisplayTime(hours, minutes);
    if(key != -1){
      switch(key){
      case 1:
        hours++;
        key = -1;
        break;
      case 2:
        minutes++;
        key = -1;
        break;
      default:
        break;
      }
    }
  }
  if(!swtkey){
    DisplayAlarm(ahours, aminutes);
    if(key != -1){
      switch(key){
      case 1:
        ahours++;
        key = -1;
        alarmTripped = false;
        break;
      case 2:
        aminutes++;
        key = -1;
        alarmTripped = false;
        break;
      default:
         break;
      }
    }
  }
  ALARM();
  if(alarmTripped){
    tripped();
  }
  if(!alarmTripped){
    //ensure the relay got shut off
    digitalWrite(RELAY, LOW);
  }
}

void countTime(){
  // One minute passes
  if(elapsedMilliseconds > 59000){
    minutes++;
    delta = millis();
  }

  // One hour passes
  if(minutes == 60){
    hours++;
  }

  // Reset 12 hour clock
  if(hours > 12){
    hours=1;
  }

  // Reset minutes to 0 after an hour
  if(aminutes > 59){
    aminutes=0;
  }

  // One hour passes
  if(aminutes == 60){
    hours++;
  }

  // Reset 12 hour clock
  if(ahours > 12){
    ahours=1;
  }

  // Reset minutes to 0 after an hour
  if(minutes > 59){
    minutes=0;
  }
}
void keyListener(){
  if(swt.pressed()){
    if(swtkey){
      swtkey = false;
    }
    else{
      swtkey = true;
    }
  }
  if(inch.pressed()){
    // Increment hours
    key = 1;
  }
  if(incm.pressed()){
    // Increment minutes
    key = 2;
  }
  if(stt.pressed()){
    // Start alarm
    key = 3;
  }
  if(stp.pressed()){
    // Stop alarm
    key = 4;
  }
}

void DisplayTime(uint16_t h, uint16_t m){
  // Format hours and seconds for clock display
  uint16_t Time = ((h*100)+m);
  if(elapsedMilliseconds %500 == 0){
    display.showNumberDecEx(Time, 0b11100000, true, 4, 0);
  }
  if(elapsedMilliseconds %1000 == 0){
    display.showNumberDec(Time, true, 4, 0);
  }
}

void DisplayAlarm(uint16_t h, uint16_t m){
    // Format hours and seconds for clock display
    uint16_t Time = ((h*100)+m);
    display.showNumberDecEx(Time, 0b11100000, true, 4, 0);
}
void ALARM(){
  if(alarmOn){
    digitalWrite(ALED, HIGH);
    if(ahours == hours and aminutes == minutes){
       alarmTripped = true;
    }
  }
  else{
    alarmTripped = false;
    digitalWrite(ALED, LOW);
  }
  switch(key){
    case 3:
      alarmOn = true;
      key = -1;
      break;
    case 4:
      alarmOn = false;
      alarmTripped = false;
      key = -1;
      break;
    default:
      break;
  }
}
void tripped(){
  /*This bit of code switches the tazer on after about 250 ms
   * and then switches it off for another 250 and then repeats
   * cycle.
   */
  switch(count){
    case 0:
      digitalWrite(RELAY, LOW);
      trippedTime = millis();
      count++;
      break;
    case 1:
      if(trippedTime - millis() > 250){
        digitalWrite(RELAY, HIGH);
        count = 0;
        break;
      }
      else{
        break;
      }
    case 2:
      if(trippedTime - millis() > 500){
         count = 0;
         trippedTime = millis();
         break;
      }
      else{
        break;
      }
    default:
      break;
  }
}