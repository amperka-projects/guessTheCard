====== Голосовой тир ======

{{ :projects:vr_overview.jpg?direct&700 }}

  * Платформы: Arduino Uno
  * Языки программирования: [[wp>Wiring_(development_platform) | Wiring (C++)]]
  * Проект на [[wpru>GitHub]]: http://github.com/amperka-projects/guessTheCard
  * Тэги: распознование речи, голосовое управление

===== Что это такое? =====

В этой статье мы расскажем о том, как можно реализовать голосовое  управление
для своего проекта. Вы сможете говорить роботу, куда ему двигаться,
включать/выключать свет, собрать замок, отпирающийчя только на ваш голос, или
голосовой тир для тренировки произношения иностранных слов. Разумеется
реализовать распознавание голоса у новичка самостоятельно не получится. Но есть
готовое решение. Мы покажем, как пользоваться этим решением, на примере
голосового тира.

Наше устройство должно будет слушать игрока, распозновать его речь и опускать
карточку на несколько секунд, как только правильно было произнесено написанное
на ней слово.

В качестве упомянутого готового решения мы возьмём плату EasyVR, которая
предоставляет простой интерфейс к сложно реализуемым функциям распознавания
голоса.

===== Что для этого необходимо? =====

{{ :projects:vr_collage.jpg?direct&700 }}

Для изготовления тира нам понадобятся:
  -[[amp>product/arduino-uno|Arduino Uno]]
  -[[amp>product/easyvr-shield|EasyVR-шилд]]
  -[[amp>product/troyka-shield|Troyka-шилд]]
  -[[amp>product/servo|Сервопривод х3]]

===== Как это собрать? =====

==== Сборка автономной части ====

  - Извлеките микросхему из платы Arduino Uno. {{ :projects:vr_build_0.jpg?direct&700 }}
  - Установите на Arduino плату Easy VR. {{ :projects:vr_build_1.jpg?direct&700 }}
  - Установите программу EasyVR commander. {{ :projects:vr_build_2.jpg?direct&700 }}
  - Создайте три команды и обучите их. {{ :projects:vr_build_3.jpg?direct&700 }}
  - Установите обратно микросхему на Arduino, установите на Easy VR плату Тройка-шилд.. {{ :projects:vr_build_sender_4.jpg?direct&700 }}

==== Сборка ретранслятора ====

  - Вставьте Ethernet шилд в Arduino Uno, установите сверху макетку и вставьте в неё беспроводной приёмник. Подключите вывод ''7'' Arduino к выводу ''2'' приёмника. {{ :projects:meteo_build_receiver_0.jpg?direct&700 }}
  - Подключите питание и землю приёмника к выводам ''GND'' и ''5V'' Arduino. {{ :projects:meteo_build_receiver_1.jpg?direct&700 }}

На этом сборка минимально функционального ретранслятора закончена. Если вы хотите установить светодиодную индикацию и звуковую сигнализацию, то выполните пункты ниже.

  - Установите светодиоды и резисторы, подключите красный светодиод к контакту ''6'', зелёный — к контакту ''5''. {{ :projects:meteo_build_receiver_2.jpg?direct&700 }}
  - Установите пьезопищалку, подключите её к контакту ''4''. {{ :projects:meteo_build_receiver_3.jpg?direct&700 }}

===== Исходный код =====

===== Код автономной части =====

<code cpp meteo_sensor.ino>
#include <SHT1x.h>
#include <LowPower_Teensy3.h>

const int rfpin = 5;

const int gnd1_pin = 10;
const int gnd2_pin = 7;

const int vcc1_pin = 11;
const int vcc2_pin = 8;

const int data_pin = 12;
const int clk_pin = 9;

SHT1x sht1x(clk_pin, data_pin);

#define _BIT(a, b) (a&(1UL<<b))
bool flag = false;
void callbackhandler() {

  //flag = true;
  
  loop_func();

}

/*****************************************************
 * The first configuration sets up Low-Power Timer and
 * Digital Pin 2 as the wake up sources. 
 *****************************************************/
 TEENSY3_LP LP = TEENSY3_LP();
sleep_block_t* LP_config;// first Hibernate configuration
void sleep_mode() {
  // configure pin 2
  pinMode(2, INPUT_PULLUP);
  // config1 struct in memory
  LP_config = (sleep_block_t*) calloc(1,sizeof(sleep_block_t));
  // OR together different wake sources
  LP_config->modules = (LPTMR_WAKE);
  // Low-Power Timer wakeup in 60 secs
  //config1->rtc_alarm = 5;
  LP_config->lptmr_timeout = 65535;
  // set pin 7 or pin 9 as wake up
  //LP_config->gpio_pin = (PIN_2);
  // user callback function
  LP_config->callback = callbackhandler;
  // go to bed
  LP.Hibernate(LP_config);
}

void rf_raise() {
  digitalWrite(rfpin, HIGH);
}

void rf_fall() {
  digitalWrite(rfpin, LOW);
}

bool rf_pin() {
  return digitalRead(rfpin);
}

void rf_putdw(unsigned long dw)
{
  byte crc, *dt = (byte*)dw;

  rf_raise();
  delayMicroseconds(70000);
  rf_fall();
  delayMicroseconds(10000);
  rf_raise();

  crc = 0x00;

  for(int i = 0; i < 32; i++) {
    if(_BIT(dw, i)) {
      delayMicroseconds(1000);
    } else {
      delayMicroseconds(3000);
    }
    rf_fall();
    delayMicroseconds(1500);
    rf_raise();
  }
  
  crc ^= dw&0xFF;
  crc ^= ((dw&0xFF00)>>8);
  crc ^= ((dw&0xFF0000)>>16);
  crc ^= ((dw&0xFF000000)>>24);

  for(int i = 0; i < 8; i++) {
    if(_BIT(crc, i)) {
      delayMicroseconds(1000);
    } else {
      delayMicroseconds(3000);
    }
    rf_fall();
    delayMicroseconds(1500);
    rf_raise();
  }

  rf_fall();
}

bool rf_getdw(unsigned long* dw) {
  unsigned long tm;

  while(!rf_pin());

  tm = millis();

  while(rf_pin());

  if(!(millis() - tm > 15 && millis() - tm < 25)) {
    return 1;
  }

  while(!rf_pin());
  tm = millis();

  for(int i = 0; i < 32; i++) {
    while(rf_pin());

    if(abs(millis() - tm) < 7) {
      *dw |= _BIT(0xFFFFFFFF, i);
    } else if(abs(millis() - tm) < 13) {
      *dw &= ~_BIT(0xFFFFFFFF, i);
    } else {
      return 2;
    }

    while(!rf_pin());
    tm = millis();
  }

  return 0;
}

void periferial_start(void) {
  pinMode(rfpin, OUTPUT);
 
  pinMode(gnd1_pin, OUTPUT);
  pinMode(gnd2_pin, OUTPUT);
  pinMode(vcc1_pin, OUTPUT);
  pinMode(vcc2_pin, OUTPUT);
  
  digitalWrite(gnd1_pin, LOW);
  digitalWrite(gnd2_pin, LOW);
  digitalWrite(vcc1_pin, HIGH);
  digitalWrite(vcc2_pin, HIGH);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
}

void periferial_stop(void) {
  pinMode(rfpin, INPUT);
 
  pinMode(gnd1_pin, INPUT);
  pinMode(gnd2_pin, INPUT);
  pinMode(vcc1_pin, INPUT);
  pinMode(vcc2_pin, INPUT);
  
  pinMode(18, INPUT_PULLUP);
  pinMode(19, INPUT_PULLUP);
  
  digitalWrite(gnd1_pin, LOW);
  digitalWrite(gnd2_pin, LOW);
  digitalWrite(vcc1_pin, LOW);
  digitalWrite(vcc2_pin, LOW);
  
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
}

void setup() {
  sleep_mode();
}

void loop() {
  if(flag) {
    flag = false;
      loop_func();  
  sleep_mode();
  }
}

void loop_func() {
  unsigned long msg;
  byte temp, humidity;
  
  periferial_start();

  delay(30);
  
  temp = (byte)(sht1x.readTemperatureC() + 40.)*2;
  humidity = (byte)sht1x.readHumidity();
  
  msg = 0;
  msg |= humidity;
  msg <<= 8;
  msg |= temp;
  
  for(int i = 0; i < 3; i++) {
    rf_putdw(msg);
  }
  
  periferial_stop();
}


</code>

===== Код платы, работающей в помещении =====

<code cpp receiver.ino>

#include <SPI.h>
#include <Ethernet.h>
 
byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0xBC, 0x71 };
 
char server[] = "narodmon.ru";
 
EthernetClient client;
 
const int rfpin = 7;
 
#define _BIT(a, b) (a&(1UL<<b))
 
 
void rf_raise() {
  digitalWrite(rfpin, HIGH);
}
 
void rf_fall() {
  digitalWrite(rfpin, LOW);
}
 
bool rf_pin() {
  return digitalRead(rfpin);
}
 
bool rf_getdw(unsigned long* dw) {
  unsigned long tm;
  byte crc, rcrc;
 
  do {
  while(!rf_pin());
  tm = micros();
  while(rf_pin());
  } while(!((micros() - tm) > 45000 && (micros() - tm) < 72000));
 
  crc = 0;
 
  while(!rf_pin());
  tm = micros();
 
  for(int i = 0; i < 32; i++) {
    while(rf_pin());
 
    if(abs(micros() - tm) < 1500) {
      *dw |= _BIT(0xFFFFFFFF, i);
    } else if(abs(micros() - tm) < 4000) {
      *dw &= ~_BIT(0xFFFFFFFF, i);
    } else {
      //Serial.println("!!!");
      return 2;
    }
 
    while(!rf_pin());
    tm = micros();
  }
 
  crc ^= (*dw)&0xFF;
  crc ^= (((*dw)&0xFF00)>>8);
  crc ^= (((*dw)&0xFF0000)>>16);
  crc ^= (((*dw)&0xFF000000)>>24);
  rcrc = 0;
 
  for(int i = 0; i < 8; i++) {
    while(rf_pin());
 
    if(abs(micros() - tm) < 1500) {
      rcrc |= _BIT(0xFFFFFFFF, i);
    } else if(abs(micros() - tm) < 4000) {
      rcrc &= ~_BIT(0xFFFFFFFF, i);
    } else {
      //Serial.println("!!!");
      return 2;
    }
 
    while(!rf_pin());
    tm = micros();
  }
 
  if(rcrc != crc) { /*Serial.print(rcrc, HEX); Serial.print(" - "); Serial.println(crc, HEX);*/ return 3; }
 
  return 0;
}
 
 
void setup() {
  pinMode(rfpin, INPUT);
  pinMode(6, OUTPUT);
 
  Serial.begin(9600);
  Serial.println("Started.");
}
 
void loop() {
  unsigned long msg;
  static unsigned long pushtimeout = 0;
  static float temp, humidity, voltage;
  int res;
 
  if((res = rf_getdw(&msg)) == 0) {      
      temp = ((float)(msg&0xFF))/2. - 40.;
      msg >>= 8;
      humidity = (float)(msg&0xFF);
      msg >>= 8;
      voltage = (float)(msg&0xFF) / 256. * 1.2 * 10 * 1.1;
 
      digitalWrite(6, HIGH);
      Serial.print("Temp: ");
      Serial.print(temp);
      Serial.print(", humidity: ");
      Serial.print(humidity);
      Serial.print(", voltage: ");
      Serial.println(voltage);
      digitalWrite(6, LOW);
  }
  else Serial.println('E');
 
  if(millis() - pushtimeout > 60000*5) {
    pushtimeout = millis();
 
    Serial.println("Starting Ethernet...");
 
    if (Ethernet.begin(mac) == 0) {
      Serial.println("Failed to configure Ethernet using DHCP");
      while(1) { }
    }
    delay(1000);
    Serial.println("connecting...");
 
    if (client.connect(server, 8283)) {
      Serial.println("connected");
 
      client.println("#90-A2-DA-0F-BC-71#AmperkaWeather#55.7466#37.6625#40.0");
 
      client.print("#90A2DA0FBC7101#");
      client.print(temp, DEC);
      client.println("#In");
 
      client.print("#90A2DA0FBC7102#");
      client.print(humidity, DEC);
      client.println("#Humidity");
      
      client.print("#90A2DA0FBC7103#");
      client.print(voltage, DEC);
      client.println("#Voltage");
 
      client.println("##");
    } 
    else {
      Serial.println("connection failed");
    }
 
    {
      unsigned long tm = millis();
 
      while(millis() - tm < 5000) {
        if (client.available()) {
          char c = client.read();
          Serial.print(c);
        }
      }
    }
 
    client.stop();
  }
}

</code>

===== Регистрация метеостанции в «Народном мониторинге» =====

Чтобы данные, передаваемые нашим устройством, корректно отображались на народном
мониторинге, необходимо выполнить следующее:

  - Установить уникальный MAC-адрес устройства.
  - Зарегистрироваться на сайте «Народного мониторинга».{{ :projects:meteo_register.png?direct&700 }}
  - Авторизоваться.
  - Открыть список датчиков и установить номиналы передаваемых данных.{{ :projects:meteo_sensorsfix.png?direct&700 }}


===== Демонстрация работы устройства =====

{{youtube>R7ILpfYvDNc?large}}

===== Что ещё можно сделать? =====

  - Мы установили только сенсор температуры и влажности. Но у Teensy остаётся ещё много свободных ножек, т.ч. можно добавить разных датчиков: освещённости, атмосферного давления, скорости ветра и т.д..
  - Teensy прямо на борту имеет часы реального времени (RTC). Для их работоспособности не хватает только кварца. Можно купить кварц на 32,768 КГц и припаять его. Тогда можно пробуждать Teensy по будильнику RTC. Достоинство в том, что можно будить устройство чаще в те часы, когда нужны более точные показания. Например, в рабочее время будить устройство каждые 5 минут, а в остальное — каждые полчаса.
