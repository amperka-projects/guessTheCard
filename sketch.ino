#include <Servo.h>
#include <EasyVR.h>


EasyVR easyvr(Serial);
Servo* srv;
 
// Номер пина для светодиода, индицирующего ожидание команды
const int LED_WAIT = 13;

// Номер группы команд
enum
{
    GROUP_MAIN = 1,
};
 
// Коды команд из группы 1
enum
{
    G1_SHIRT = 0,
    G1_PEAR = 1,
    G1_ORANGE = 2,
};
 
// Соответствие команд индексам в массиве серв srv
enum
{
    // Если значение элемента перечисления не задано, то оно принимается
    // равным <значение предыдущего элемента>+1. Если элемент первый, то ему
    // присваивается значение 0.
    SRV_SHIRT,
    SRV_PEAR,
    SRV_ORANGE,
    // Эту константу держите всегда последней и не меняйте её значение:
    SRV_COUNT
};

// Соостветствие серв пинам
enum
{
    SRV_SHIRT_PIN  = 3,
    SRV_PEAR_PIN   = 5,
    SRV_ORANGE_PIN = 6
};
 
 
void setup(void)
{
    Serial.begin(9600);
   
    // Переводим на запись порт для индикации
    // активности микрофона
    pinMode(LED_WAIT, OUTPUT);
    digitalWrite(LED_WAIT, LOW);

    // Ожидание соединения с платой
    while(!easyvr.detect()) delay(1000);

    // Установка таймаута на распознавание
    easyvr.setTimeout(5);

    // Выбор языка (Английский)
    easyvr.setLanguage(EasyVR::ENGLISH);

    // Конфигурируем сервы
    srv = new Servo[SRV_COUNT];

    srv[  SRV_SHIRT ].attach(  SRV_SHIRT_PIN );
    srv[ SRV_ORANGE ].attach( SRV_ORANGE_PIN );
    srv[   SRV_PEAR ].attach(   SRV_PEAR_PIN );
   
    // По очереди протестируем все сервы
    srv[SRV_SHIRT].write(90);
    delay(1000);
    srv[SRV_SHIRT].write(0);
   
    delay(1000);
   
    srv[SRV_ORANGE].write(90);
    delay(1000);
    srv[SRV_ORANGE].write(0);
   
    delay(1000);
   
    srv[SRV_PEAR].write(90);
    delay(1000);
    srv[SRV_PEAR].write(0);
}
 

// Данная функция вызывается в случае успешного
// распознавания фразы
void action(int8_t group, int8_t idx)
{
    // Активируем серву, соответствующую распознанному коду
    switch (group)
    {
    case GROUP_MAIN:
        switch (idx)
        {
        case G1_SHIRT:
            srv[SRV_SHIRT].write(90);
            delay(1000);
            srv[SRV_SHIRT].write(0);
            break;
        case G1_PEAR:
            srv[SRV_PEAR].write(90);
            delay(1000);
            srv[SRV_PEAR].write(0);
            break;
        case G1_ORANGE:
            srv[SRV_ORANGE].write(90);
            delay(1000);
            srv[SRV_ORANGE].write(0);
            break;
        }
        break;
    }
}
 

void loop(void)
{
    int8_t idx;

    // Зажигаем светодиод для индикации режима ожидания команды
    digitalWrite(LED_WAIT, HIGH);

    // Запускаем процедуру распознавания
    easyvr.recognizeCommand(GROUP_MAIN);

    // Ожидаем окончание процесса
    while(!easyvr.hasFinished());
   
    // Тушим светодиод - либо таймаут, либо команда распознана
    digitalWrite(LED_WAIT, LOW);
    
    // Если убрать эту задержку, то мы перестанем видеть, когда заканчивается
    // один период ожидания команды и начинается второй. Если произнесение команды
    // придётся на границу двух периодов, то распознавание закончится неудачей.
    delay(500);

    // Получаем код распознанной команды
    idx = easyvr.getCommand();

    if(idx >= 0)
    {
        // Выполним действие по команде
        action(GROUP_MAIN, idx);
    }
    else
    {
        // Ошибка распознавания или таймаут
    }
}
