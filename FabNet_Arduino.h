#ifndef FABNET_ARDUINO_H
#define FABNET_ARDUINO_H

#include <inttypes.h>

#include <Arduino.h>
#include <SoftwareSerial.h>

#include "FabNet_Protocol.h"

//Переменные двухканального блока реле
#define RELAY_BOARD_CHANNEL1_VAR_ID 0x00
#define RELAY_BOARD_CHANNEL2_VAR_ID 0x01

//Управление реле

//Включение реле
#define RELAY_BOARD_CHANNEL_ON    true
//Выключение реле
#define RELAY_BOARD_CHANNEL_OFF    false

class FabNet_Arduino {
  private:
    //Выводы для управления драйвером сети FabNet
    uint8_t _re_pin; 
    uint8_t _de_pin;

    //Вывод для управления светодиодом индикации сетевой активности
    uint8_t _led_pin; 

    //Последователный порт для доступа к сети FabNet
    SoftwareSerial _serial;

    //Идентификатор контроллера в сети FabNet
    uint16_t _ctrl_dev_id;

    //Расчет контрольной суммы пакета
    uint8_t rs485_calc_crc(uint8_t * data, uint8_t size);

    //Прием данных из сети RS485
    bool rs485_recv(void * data, uint8_t size);

    //Передача данных в сеть RS485
    bool rs485_send(void * data, uint8_t size);

    //Мигание светодиодом активности сети
    void net_led_blink(void);

    //Формирования пакета запроса чтения переменной
    void create_get_var_pkg(t_rs485_pkg * rs485_pkg, uint16_t dev_id, uint8_t var_id);
    //Формирования пакета запроса записи переменной
    void create_set_var_pkg(t_rs485_pkg * rs485_pkg, uint16_t dev_id, uint8_t var_id, uint8_t var_type, t_var_value var_val);
    //Разбор oтвета на запрос чтения переменной
    bool parse_var_ack_pkg(t_rs485_pkg * rs485_pkg, uint16_t dev_id, uint8_t var_id, uint8_t vat_type, t_var_value * var_val);

    //Разбор ответного пакета
    bool parse_ack_pkg(t_rs485_pkg * rs485_pkg, uint16_t dev_id);
  public:
    //Конструктор класса
    FabNet_Arduino(uint8_t re_pin, uint8_t de_pin, uint8_t ro_pin, uint8_t di_pin, uint8_t led_pin);

    //Настройка аппаратной конфигурации
    void begin(void);
    
    //Чтение вещественной переменной
    bool read_float_var(uint16_t dev_id, uint8_t var_id, float * var_val);
    //Чтение целочисленной переменной со знаком
    bool read_int_var(uint16_t dev_id, uint8_t var_id, int32_t * var_val);
    //Чтение целочисленной перемпенной без знака
    bool read_uint_var(uint16_t dev_id, uint8_t var_id, uint32_t * var_val);
    //Чтение логической переменной
    bool read_bool_var(uint16_t dev_id, uint8_t var_id, bool * var_val);

    //Запись логической переменной
    bool write_bool_var(uint16_t dev_id, uint8_t var_id, bool var_val);

    //Задание идентификатора контроллера в сети FabNet
    void set_ctrl_id(uint16_t ctrl_dev_id);
};

#endif