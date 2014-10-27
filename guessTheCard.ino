#include <Servo.h>

#define SERVO_DOWN   10
#define SERVO_UP     100

#define SERVO_COUNT  4

#define NONE         -1

Servo servos[SERVO_COUNT];

byte servoPins[SERVO_COUNT] = {4, 5, 6, 7};

byte currentQuestion;
byte currentAnswer = NONE;



#define START_SAY_LED  11
#define WIN_LED        10



#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#include "Platform.h"
#include "SoftwareSerial.h"
#ifndef CDC_ENABLED
// Shield Jumper on SW
SoftwareSerial port(12, 13);
#else
// Shield Jumper on HW (for Leonardo)
#define port Serial1
#endif
#else // Arduino 0022 - use modified NewSoftSerial
#include "WProgram.h"
#include "NewSoftSerial.h"
NewSoftSerial port(12, 13);
#endif

#include "EasyVR.h"

EasyVR easyvr(port);

//Groups and Commands
enum Groups
{
  GROUP_1  = 1,
};

enum Group1
{
  G1_SHIRT  = 0,
  G1_PANTS  = 1,
  G1_PEAR   = 2,
  G1_ORANGE = 3,
};


EasyVRBridge bridge;

int8_t group, idx;



void setup()
{
  randomSeed(analogRead(A0));
  currentQuestion = random(SERVO_COUNT);

  for (int i = 0; i < SERVO_COUNT; ++i) {
    servos[i].attach(servoPins[i]);
    servos[i].write(SERVO_DOWN);
  }


  pinMode(START_SAY_LED, OUTPUT);
  pinMode(WIN_LED, OUTPUT);


#ifndef CDC_ENABLED
  // bridge mode?
  if (bridge.check())
  {
    cli();
    bridge.loop(0, 1, 12, 13);
  }
  // run normally
  Serial.begin(9600);
  Serial.println("Bridge not started!");
#else
  // bridge mode?
  if (bridge.check())
  {
    port.begin(9600);
    bridge.loop(port);
  }
  Serial.println("Bridge connection aborted!");
#endif
  port.begin(9600);

  while (!easyvr.detect())
  {
    Serial.println("EasyVR not detected!");
    delay(1000);
  }

  easyvr.setPinOutput(EasyVR::IO1, LOW);
  Serial.println("EasyVR detected!");
  easyvr.setTimeout(5);
  easyvr.setLanguage(0);

  group = GROUP_1;// EasyVR::TRIGGER; //<-- start group (customize)
}


void action();

void loop()
{
  easyvr.setPinOutput(EasyVR::IO1, HIGH); // LED on (listening)
  digitalWrite(START_SAY_LED, LOW);      // LED on (listening)
  digitalWrite(WIN_LED, LOW);      // LED on (WINNER)

  Serial.print("Say a command in Group ");
  Serial.println(group);
  easyvr.recognizeCommand(group);

  do
  {
    checkQuestion();
    // can do some processing while waiting for a spoken command
  }
  while (!easyvr.hasFinished());

  easyvr.setPinOutput(EasyVR::IO1, LOW); // LED off

  idx = easyvr.getWord();
  if (idx >= 0)
  {
    // built-in trigger (ROBOT)
    // group = GROUP_X; <-- jump to another group X
    return;
  }
  idx = easyvr.getCommand();
  if (idx >= 0)
  {
    // print debug message
    uint8_t train = 0;
    char name[32];
    Serial.print("Command: ");
    Serial.print(idx);
    if (easyvr.dumpCommand(group, idx, name, train))
    {
      Serial.print(" = ");
      Serial.println(name);
    }
    else
      Serial.println();
    easyvr.playSound(0, EasyVR::VOL_FULL);
    // perform some action
    action();
  }
  else // errors or timeout
  {
    digitalWrite(START_SAY_LED, HIGH);

    if (easyvr.isTimeout())
      Serial.println("Timed out, try again...");
    int16_t err = easyvr.getError();
    if (err >= 0)
    {
      Serial.print("Error ");
      Serial.println(err, HEX);
    }
  }
}

void action()
{
  switch (group)
  {
    case GROUP_1:
      switch (idx)
      {
        case G1_SHIRT:
          digitalWrite(WIN_LED, HIGH);      // LED on (WINNER)

          currentAnswer = 0;

          // write your action code here
          // group = GROUP_X; <-- or jump to another group X for composite commands
          break;
        case G1_PANTS:

          digitalWrite(WIN_LED, HIGH);      // LED on (WINNER)

          currentAnswer = 1;

          // write your action code here
          // group = GROUP_X; <-- or jump to another group X for composite commands
          break;
        case G1_PEAR:
          digitalWrite(WIN_LED, HIGH);      // LED on (WINNER)
          currentAnswer = 2;

          // write your action code here
          // group = GROUP_X; <-- or jump to another group X for composite commands
          break;
        case G1_ORANGE:
          digitalWrite(WIN_LED, HIGH);      // LED on (WINNER)
          currentAnswer = 3;

          // write your action code here
          // group = GROUP_X; <-- or jump to another group X for composite commands
          break;
      }
      break;
  }
}


void checkQuestion()
{
  if (currentQuestion == currentAnswer) {
    servos[currentQuestion].write(SERVO_DOWN);
    delay(500);

    while (currentAnswer == currentQuestion)
    {
      currentQuestion = random(SERVO_COUNT);

      Serial.println(currentQuestion);
    }

  }

  servos[currentQuestion].write(SERVO_UP);

}
