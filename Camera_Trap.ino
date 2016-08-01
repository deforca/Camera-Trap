/*
  Camera Trap for Nikkon D60
  
  We have a motion sensor connected to pin #7, and an IR LED connected to pin #3
  When motion is detected, we send an IR signal corresponding to the Nikkon's IR shutter trigger command, and we also make the Arduino internal led blink
  Every 10 minutes (or 10 min. after the last motion detection) we send the camera an IR shutter trigger command, to avoid the Nikkon's IR mode to time out

  author Alberto Amengual
  modified JUNE 28 2016 by Alberto Amengual
 */

// The TimerOne library provides functions to use the Timer1 timer in the Arduino
#include <TimerOne.h>

// The IRremote library provides functions to send IR signals
#include <IRremote.h>
#include <IRremoteInt.h>

const byte ledPin = 13;
const byte pirPin = 7;
byte motionOngoing = LOW;

// forcePic controls when it's time to force the camera to take a pic (to avoid IR mode time out)
volatile byte forcePic = LOW;
// Because the TimerOne library allows only short timers, we use fiveSecLoopCounter to add time up and have longer timers
volatile int fiveSecLoopCounter = 0;

// IR Shutter signal pulses for the Nikkon, as found on bigmike.it
const unsigned int rawCodes[] = {2000, 27830, 400, 1580, 400, 3580, 400};
const unsigned int codeLen = 7;
  
// On the Arduino UNO, IRsend will use PIN 3 for PWM
IRsend irsend;

void takePic(void)
{
  fiveSecLoopCounter++;
  // Every 10min (120 x 5 sec = 600 sec) force the camera to take a pic
  if (fiveSecLoopCounter == 120){
     fiveSecLoopCounter = 0;
     forcePic = HIGH;
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);

  //Serial.begin(9600);

  // Timer1 is used to force camera to take a pic every 10 min, so that the Nikkon's remote mode doesn't time out (after 15 min).
  Timer1.initialize(5000000); // The TimerOne library timers have a limit of aprox 8 seconds; we're using 5 sec timers
  Timer1.attachInterrupt(takePic); 
}

void loop() {
  motionOngoing = digitalRead(pirPin);
  digitalWrite(ledPin, motionOngoing);
  
  if (motionOngoing || forcePic){
        // Before taking a picture, reset all the variables that control the automatic taking of pics
        forcePic = LOW;
        fiveSecLoopCounter = 0;
        // Timer1.restart() for some reason doesn't work, so we initialize Timer1 again and attachInterrupt
        Timer1.initialize(5000000);
        Timer1.attachInterrupt(takePic);

        // Send the IR sequence corresponding to the Nikkon's IR shutter command
        // The third parameter means a PWM at 38kHz
        irsend.sendRaw(rawCodes, codeLen, 38);
        //Serial.println("Force pic");
        // DO NOT REMOVE THE DELAY BELOW. Sending IR shutter commands at full speed seems to prevent the camera from properly processing the commands.
        delay(1000);
  }
}

