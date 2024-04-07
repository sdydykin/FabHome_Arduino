#include <Ds1302.h>
#include <SoftwareSerial.h>

#include "FabNet_Arduino.h"

//Выводы подключения RTC модуля
#define RTC_CE_PIN    8
#define RTC_DAT_PIN   9
#define RTC_CLK_PIN   10

//Выводы подключения RS485 интерфейса (сеть FabNet)
#define RS485_REn_PIN 14
#define RS485_DE_PIN  15
#define RS485_RO_PIN  16
#define RS485_DI_PIN  17

//Выводы подключения светодиодов
#define LED_PWR_PIN   18
#define LED_NET_PIN   19

//Последовательный интерфейс для доступа к сети FabNet
SoftwareSerial rs485_uart(RS485_RO_PIN, RS485_DI_PIN);
//Сеть FabNet
FabNet_Arduino fab_net(rs485_uart, RS485_REn_PIN, RS485_DE_PIN);
//RTC модуль
Ds1302 rtc(RTC_CE_PIN, RTC_CLK_PIN, RTC_DAT_PIN);

//Нпачальная инициализация переферии
void setup() {
    //Старт пользовательского последовательного порта
    Serial.begin(9600);

    //Старт последовательного интерфейса для доступа к сети FabNet
    rs485_uart.begin(19200);

    //Старт сети FabNet
    fab_net.begin();

    //Старт RTC модуля
    rtc.init();

    //Настройка светодиодов
    pinMode(LED_PWR_PIN, OUTPUT);
    pinMode(LED_NET_PIN, OUTPUT);

    //Включаем светодиод индикации питания
    digitalWrite(LED_PWR_PIN, HI);
}

//Основной цикл программы
void loop() {
  
}
