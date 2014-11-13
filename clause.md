====== Голосовой тир ======

{{ :projects:vr_overview.jpg?direct&700 }}

  * Платформы: Arduino Uno
  * Языки программирования: [[wp>Wiring_(development_platform) | Wiring (C++)]]
  * Проект на [[wpru>GitHub]]: http://github.com/amperka-projects/guessTheCard
  * Тэги: распознование речи, голосовое управление

===== Что это такое? =====

В этой статье мы расскажем о том, как собрать тир с голосовым управлением. При
помощи такого тира можно обучать людей произношению иностранных слов или их
запоминанию.

Наше устройство должно будет слушать игрока, распознавать его речь и опускать
карточку на несколько секунд, как только правильно было произнесено написанное
на ней слово.

За основу мы возьмём австрийское решение для распознавания голосовых команд —
EasyVR 2.0.

===== Что для этого необходимо? =====

{{ :projects:vr_collage.jpg?direct&700 }}

Для изготовления тира с тремя карточками нам понадобятся:
  - [[amp>product/arduino-uno|Arduino Uno]]
  - [[amp>product/easyvr-shield|EasyVR-шилд]]
  - [[amp>product/arduino-troyka-shield|Troyka-шилд]]
  - [[amp>product/servo-fs90|Микросервопривод FS90 ×3 шт]]
  - Карточки с изображениями или словами ×3 шт

===== Как это собрать? =====

  - Извлеките микросхему из платы Arduino Uno. Мы будем использовать плату как переходник USB → UART для программирования EasyVR. {{ :projects:vr_build_0.jpg?direct&700 }}
  - Установите на плату Arduino шилд EasyVR, перемычку на EasyVR в положение ''PC''. {{ :projects:vr_build_1.jpg?direct&700 }}
  - Установите программу, подключите Arduino к компьютеру [[http://www.veear.eu/downloads/|EasyVR Commander]]. {{ :projects:vr_build_2.jpg?direct&700 }}
  - Создайте три команды и обучите их. {{ :projects:vr_build_3.jpg?direct&700 }}
  - Установите обратно микросхему на Arduino, прошейте плату Arduino скетчем, приведённым ниже. {{ :projects:vr_build_4.jpg?direct&700 }}
  - Установите на Easy VR плату Тройка-шилд. Перемычка на EasyVR должна стоять в положении «HW». {{ :projects:vr_build_5.jpg?direct&700 }}
  - Подключите сервоприводы к тройка-шилду. {{ :projects:vr_build_6.jpg?direct&700 }}

===== Исходный код =====

<code cpp guessTheCard.ino>
#include <Wire.h>

/**
  EasyVR Tester
  
  Dump contents of attached EasyVR module
  and exercise it with playback and recognition.
  
  Serial monitor can be used to send a few basic commands:
  '?' - display the module setup
  'l' - cycles through available languages
  'c' - cycles through available command groups
  'b' - cycles through built-in word sets
  'g' - cycles through custom grammars
  's123' - play back sound 123 if available (or beep)
  'd0123456789ABCD*#' - dials the specified number ('_' is dial tone)
  'k' - starts detection of tokens
  '4' or '8' - sets token length to 4 or 8 bits
  'n123' - play back token 123 (not checked for validity)
  
  With EasyVR Shield, the green LED is ON while the module
  is listening (using pin IO1 of EasyVR).
  Successful recognition is acknowledged with a beep.
  Details are displayed on the serial monitor window.

**
  Example code for the EasyVR library v1.4
  Written in 2014 by RoboTech srl for VeeaR <http:://www.veear.eu> 

  To the extent possible under law, the author(s) have dedicated all
  copyright and related and neighboring rights to this software to the 
  public domain worldwide. This software is distributed without any warranty.

  You should have received a copy of the CC0 Public Domain Dedication
  along with this software.
  If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
  #include "Platform.h"
  #include "SoftwareSerial.h"
#ifndef CDC_ENABLED
  // Shield Jumper on SW
  //SoftwareSerial port(12,13);
  #define port Serial
#else
  // Shield Jumper on HW (for Leonardo)
  #define port Serial1
#endif
#else // Arduino 0022 - use modified NewSoftSerial
  //#include "WProgram.h"
  //#include "NewSoftSerial.h"
  //NewSoftSerial port(12,13);
#endif

#include "EasyVR.h"

#include "Servo.h"

EasyVR easyvr(port);

int8_t bits = 4;
int8_t set = 0;
int8_t group = 1;
uint32_t mask = 0;  
uint8_t train = 0;
uint8_t grammars = 0;
int8_t lang = 0;
char name[33];
bool useCommands = true;
bool useTokens = false;

EasyVRBridge bridge;

Servo* srv;

void setup()
{
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  
#ifndef CDC_ENABLED
  // bridge mode?
  if (bridge.check())
  {
    cli();
    bridge.loop(0, 1, 12, 13);
  }
  // run normally
  //Serial.begin(9600);
  //Serial.println("Bridge not started!");
#else
  // bridge mode?
  if (bridge.check())
  {
    port.begin(9600);
    bridge.loop(port);
  }
  //Serial.println("Bridge connection aborted!");
#endif
  port.begin(9600);

  while (!easyvr.detect())
  {
    //Serial.println("EasyVR not detected!");
    delay(1000);
  }

  easyvr.setPinOutput(EasyVR::IO1, LOW);
  //Serial.print("EasyVR detected, version ");
  //Serial.println(easyvr.getID());
  easyvr.setTimeout(5);
  lang = EasyVR::ENGLISH;
  easyvr.setLanguage(lang);
  
  int16_t count = 0;
  
  //Serial.print("Sound table: ");
  if (easyvr.dumpSoundTable(name, count))
  {
    //Serial.println(name);
    //Serial.print("Sound entries: ");
    //Serial.println(count);
  }
  else
    //Serial.println("n/a");

  //Serial.print("Custom Grammars: ");
  grammars = easyvr.getGrammarsCount();
  if (grammars > 4)
  {
    //Serial.println(grammars - 4);
    for (set = 4; set < grammars; ++set)
    {
      //Serial.print("Grammar ");
      //Serial.print(set);

      uint8_t flags, num;
      if (easyvr.dumpGrammar(set, flags, num))
      {
        //Serial.print(" has ");
        //Serial.print(num);
        //if (flags & EasyVR::GF_TRIGGER)
          //Serial.println(" trigger");
        //else
          //Serial.println(" command(s)");
      }
     /// else
       // Serial.println(" error");
        
      for (int8_t idx = 0; idx < num; ++idx)
      {
        //Serial.print(idx);
        //Serial.print(" = ");
        if (!easyvr.getNextWordLabel(name))
          break;
        //Serial.println(name);
      }
    }
  }
  else
    /*Serial.println("n/a")*/;
  
  if (easyvr.getGroupMask(mask))
  {
    uint32_t msk = mask;  
    for (group = 0; group <= EasyVR::PASSWORD; ++group, msk >>= 1)
    {
      if (!(msk & 1)) continue;
      if (group == EasyVR::TRIGGER)
        /*Serial.print("Trigger: ")*/;
      else if (group == EasyVR::PASSWORD)
       /* Serial.print("Password: ")*/;
      else
      {
        /*Serial.print("Group ");
        Serial.print(group);
        Serial.print(" has ");*/
      }
      count = easyvr.getCommandCount(group);
   //   Serial.print(count);
      if (group == 0)
        /*Serial.println(" trigger")*/;
      else
        /*Serial.println(" command(s)")*/;
      for (int8_t idx = 0; idx < count; ++idx)
      {
        if (easyvr.dumpCommand(group, idx, name, train))
        {
          /*Serial.print(idx);
          Serial.print(" = ");
          Serial.print(name);
          Serial.print(", Trained ");
          Serial.print(train, DEC);*/
          if (!easyvr.isConflict())
            /*Serial.println(" times, OK")*/;
          else
          {
            int8_t confl = easyvr.getWord();
            if (confl >= 0)
              /*Serial.print(" times, Similar to Word ")*/;
            else
            {
              confl = easyvr.getCommand();
              /*Serial.print(" times, Similar to Command ")*/;
            }
            /*Serial.println(confl)*/;
          }
        }
      }
    }
  }
  group = 1;
  mask |= 1; // force to use trigger
  useCommands = (mask != 1);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  
  srv = new Servo[3];
  
  srv[0].attach(3);
  srv[0].write(0);

  srv[1].attach(5);
  srv[1].write(0);

  srv[2].attach(6);
  srv[2].write(0);
  
  srv[0].write(90);
  delay(2000);
  srv[0].write(0);
  
  delay(1000);
  
  srv[1].write(90);
  delay(2000);
  srv[1].write(0);
  
  delay(1000);
  
  srv[2].write(90);
  delay(2000);
  srv[2].write(0);
}

const char* ws0[] =
{
  "ROBOT",
};
const char* ws1[] =
{
  "ACTION",
  "MOVE",
  "TURN",
  "RUN",
  "LOOK",
  "ATTACK",
  "STOP",
  "HELLO",
};
const char* ws2[] =
{
  "LEFT",
  "RIGHT",
  "UP",
  "DOWN",
  "FORWARD",
  "BACKWARD",
};
const char* ws3[] =
{
  "ZERO",
  "ONE",
  "TWO",
  "THREE",
  "FOUR",
  "FIVE",
  "SIX",
  "SEVEN",
  "EIGHT",
  "NINE",
  "TEN",
};

const char** ws[] = { ws0, ws1, ws2, ws3 };

void loop()
{
  //checkMonitorInput();
  
  easyvr.recognizeCommand(group);
  
  do
  {
  }
  while (!easyvr.hasFinished());
  
  easyvr.setPinOutput(EasyVR::IO1, LOW); // LED off

  int16_t idx;
  idx = easyvr.getCommand();
  
    if (idx >= 0)
    {
      //Serial.print("Command: ");
      //Serial.print(easyvr.getCommand());
      //if (easyvr.dumpCommand(group, idx, name, train))
      //{
        //Serial.print(" = ");
        //Serial.println(name);
        
        if(idx < 3) {
        pinMode(13, OUTPUT);
        digitalWrite(13, LOW);
        
        srv[idx].write(90);
        delay(1000);
        srv[idx].write(0);
        } else {
          pinMode(13, OUTPUT);
        digitalWrite(13, HIGH);
        }
      //}
      //else
        /*Serial.println()*/;
      // ok, let's try another group
      /*do
      {
        //group++;
        //if (group > EasyVR::PASSWORD)
        //  group = 0;
      } while (!((mask >> group) & 1));
      easyvr.playSound(0, EasyVR::VOL_FULL);*/
    }
    else // errors or timeout
    {
      if (easyvr.isTimeout())
        /*Serial.println("Timed out, try again...")*/;
      int16_t err = easyvr.getError();
      if (err >= 0)
      {
 //       Serial.print("Error 0x");
   //     Serial.println(err, HEX);
           pinMode(13, OUTPUT);
           digitalWrite(13, HIGH);
      }
    }
}

</code>

===== Демонстрация работы устройства =====

{{youtube>R7ILpfYvDNc?large}}

===== Что ещё можно сделать? =====

Возможности платы EasyVR — огромны. Мы показали лишь самый простой пример
использования этого модуля. При этом было задействовано едва ли 30%
возможностей. Помимо распознавания голоса на английском языке модуль способен:

  - Распознавать команды на некоторых других языках (Немецкий, Французский, Итальянский, Испанский, Корейский и Японский). Это даёт возможность строить устройства для обучения произношению иностранных слов на всех перечисленных языках.
  - Воспользовавшись режимом «
