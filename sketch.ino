#include <Servo.h>
#include <EasyVR.h>

// Номер пина для светодиода, индицирующего ожидание команды
#define LED_WAIT    13
// Сколько у нас мишеней?
#define SERVO_COUNT 3
// Мы будем использовать только 1 группу команд EasyVR
#define GROUP_MAIN  1

EasyVR easyvr(Serial);

// Массив объектов для серво
Servo srv[SERVO_COUNT];

// На какие пины подключаем сервы с мишенями
byte srvPins[SERVO_COUNT] = {3, 5, 6};


void setup()
{
    Serial.begin(9600);

    // Переводим на запись порт для индикации
    // активности микрофона
    pinMode(LED_WAIT, OUTPUT);
    digitalWrite(LED_WAIT, LOW);

    // Ожидание соединения с платой
    while (!easyvr.detect()) delay(1000);

    // Установка таймаута на распознавание
    easyvr.setTimeout(5);

    // Выбор языка (Английский)
    easyvr.setLanguage(EasyVR::ENGLISH);

    // Конфигурируем сервы
    for (byte i = 0; i < SERVO_COUNT; i++) {
        // подключаем серву к пину
        srv[i].attach(srvPins[i]);

        // проверяем работоспособность серво
        srv[i].write(90);
        delay(1000);
        srv[i].write(0);
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
    while (!easyvr.hasFinished()) {
    }

    // Тушим светодиод - либо таймаут, либо команда распознана
    digitalWrite(LED_WAIT, LOW);

    // Если убрать эту задержку, то мы перестанем видеть, когда заканчивается
    // один период ожидания команды и начинается второй. Если произнесение команды
    // придётся на границу двух периодов, то распознавание закончится неудачей.
    delay(500);

    // Получаем код распознанной команды
    idx = easyvr.getCommand();

    if (idx >= 0) {
        // В зависимости от номера распознаной команды,
        // поднимаем соответствующую серву
        srv[idx].write(90);
        delay(1000);
        srv[idx].write(0);
    } else {
        // Если хотите отреагировать на ошибку распознавания,
        // вставьте код обработки сюда
    }
}
