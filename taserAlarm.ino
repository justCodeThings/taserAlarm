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
 * Button to write clock and alarm to EEPROM = 4
 */
 /*Also. Fucking remember that the documentation for Button.h 
  * on arduino playground is utter horsehsit.  
  Don't even bother reading it.*/
#include <EEPROM.h>
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
//P.S. -1 equates to null in this case as 0 is a valid integer around these parts
int16_t key = -1;

Button swt = Button(11);
Button inch = Button(10);
Button incm = Button(9);
Button stt = Button(8);
Button stp = Button(7);
Button wrt = Button(4);

// Get saved Time from EEPROM and reconstruct from 8bit int to 16 bit
uint16_t loadT = ((uint16_t)EEPROM.read(0) << 8) | EEPROM.read(1);
uint16_t loadA = ((uint16_t)EEPROM.read(2) << 8) | EEPROM.read(3);

// Unformat time saved to EEPROM back to minutes and hours
// Clock time
uint16_t hours;
uint16_t minutes;

// Alarm time
uint16_t ahours;
uint16_t aminutes;

// Initialize Time
uint16_t Time;
uint16_t aTime;

// Alarm booleans
bool alarmOn = false;
bool alarmTripped = false;

// Main alarm clock time
uint32_t delta = millis();
uint32_t elapMillis = 0;

TM1637Display display = TM1637Display(CLK, DIO);

void setup() {
  swt.begin();
  inch.begin();
  incm.begin();
  stt.begin();
  stp.begin();
  wrt.begin();

   /* If the EEPROM has never been written to before, it should
   *  contain the value 255 in each address.  So when we append
   *  two bytes we will get ffff hex, which run through our 
   *  arithmetic will result in a floating point number, upgrading
   *  our int to a float as C++ does; something I really  don't want.  
   *  This if statement is an attempt to protect the program from this
   *  on a first time EEPROM read.
   */
  if(loadT != 0xffff){
    hours =  loadT / 100;
    minutes = loadT - (hours * 100);
    
    // Alarm time
    ahours = loadA / 100;
    aminutes = loadA - (loadA * 100);
  }
  else{
    hours = 12;
    minutes = 0;
    ahours = 12;
    aminutes = 0;
  }
  
  pinMode(ALED, OUTPUT);
  pinMode(RELAY, OUTPUT);

  // Intialize the display
  display.clear();
  display.setBrightness(0);
}

void loop() {
  tick();
  keyListener();
  DisplayTime();
  AlarmHandler();
}

// Set default values
void DisplayTime(bool = true);
void adjustClock(uint16_t = hours, uint16_t = minutes);

void tick(){
  // Count the time in milliseconds
  elapMillis = millis() - delta;
  
  if(elapMillis > 60000){
    minutes++;
    delta = millis();
  }

  if(minutes > 59){
    minutes=0;
    hours++;
  }

  if(aminutes > 59){
    aminutes=0;
    hours++;
  }

  if(hours > 12){
    hours = 1;
  }

  if(ahours > 12){
    ahours = 1;
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
    adjustClock();
  }
  if(incm.pressed()){
    // Increment minutes
    key = 2;  
    adjustClock();
  }
  if(stt.pressed()){
    // Start alarm
    key = 3;
  }
  if(stp.pressed()){
    // Stop alarm
    key = 4;
  }
  if(inch.released() || incm.released()){
    key = 5;
  }
  if(wrt.pressed()){
    // Break 16bit times into 8bit for EEPROM storage
    uint8_t TimeB = (uint8_t)Time;
    uint8_t TimeA = (uint8_t)(Time >> 8);
    uint8_t aTimeB = (uint8_t)aTime;
    uint8_t aTimeA = (uint8_t)(aTime >> 8);
    uint32_t vals[] = {TimeA, TimeB, aTimeA, aTimeB};
    for(uint16_t i = 0; i < 4; i++){
      EEPROM.write(i, vals[i]);
    }
  }
}
void DisplayTime(bool blink){
  Time = ((hours*100)+minutes);
  aTime = ((ahours*100)+minutes);
  if(swtkey){
    if(elapMillis %500 == 0){
    display.showNumberDecEx(Time, 0b11100000, true, 4, 0);
    }
    if(elapMillis %1000 == 0){
      display.showNumberDec(Time, true, 4, 0);
    }
    if(!blink){
      display.showNumberDec(Time, true, 4, 0);
    }
  }
  else{
    display.showNumberDecEx(aTime, 0b11100000, true, 4, 0);
  }
}
void AlarmHandler(){
  if(alarmOn){
    digitalWrite(ALED, HIGH);
    if(ahours == hours and aminutes == minutes){
       alarmTripped = true;
    }
  }
  switch(key){
    case 3:
      alarmOn = true;
      key = -1;
      break;
    case 4:
      alarmOn = false;
      alarmTripped = false;
      digitalWrite(ALED, LOW);
      key = -1;
      break;
    default:
      break;
  }
  if(alarmTripped){
    if(elapMillis %3000 == 0){
      /* I use direct port manipulation to control the 
       *  relay in order to cut down on time spent making 
       *  sure the relay is truly off.  I use a hexidecimal 
       *  mask in order to make sure only pin 6 is switched
       *  low. Please note that these ports will only
       *  work on the Arduino UNO board.
       */
       // Set pin 6 HIGH
      PORTD = PORTD | 0x40; //0x40 mask.  0x40 = 0100 0000
    }
    if(elapMillis %3010 == 0){
      // Set pin 6 LOW
      PORTD = PORTD & 0xbf; //0xbf mask. 0xbf = 1011 1111
    }
  }
  if(!alarmTripped){
    //ensure the relay got shut off
    PORTD = PORTD & 0xbf;
  }
}
void adjustClock(uint16_t thours, uint16_t tminutes){
  if(!swtkey){
    adjustClock(ahours, aminutes);
  }
  switch(key){
    case 1:
      thours++;
      if(thours > 12){
        thours = 0;
      }
      DisplayTime(false);
      delay(500);
      break;
    case 2:
      tminutes++;
      if(tminutes > 59){
        tminutes = 0;
      }
      DisplayTime(false);
      break;
    case 5:
      if(swtkey){
        hours = thours;
        minutes = tminutes;
      }
      else{
        ahours = thours;
        aminutes = tminutes;
      }
    default:
      break;
  }
}