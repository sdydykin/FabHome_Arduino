#ifndef FABNET_ARDUINO_H
#define FABNET_ARDUINO_H

#include <Arduino.h>
#include <SoftwareSerial.h>

class FabNet_Arduino {
  private:
    //Выводы для управления драйвером сети FabNet
    unsigned char _re_pin; 
    unsigned char _de_pin;

    //Последователный порт для доступа к сети FabNet
    SoftwareSerial& _serial;

    //Идентификатор контроллера в сети FabNet
    unsigned int _ctrl_dev_id;
  public:
    //Конструктор класса
    FabNet_Arduino(SoftwareSerial& serial, unsigned char re_pin, unsigned char de_pin);

    //Настройка аппаратной конфигурации
    void begin(void);
    
    //Чтение вещественной переменной
    bool read_float_var(unsigned int dev_id, unsigned char var_id, float * var_val);
    //Чтение целочисленной переменной со знаком
    bool read_int_var(unsigned int dev_id, unsigned char var_id, long * var_val);
    //Чтение целочисленной перемпенной без знака
    bool read_uint_var(unsigned int dev_id, unsigned char var_id, unsigned long * var_val);
    //Чтение логической переменной
    bool read_bool_var(unsigned int dev_id, unsigned char var_id, bool * var_val);

    //Запись логической переменной
    bool write_bool_var(unsigned int dev_id, unsigned char var_id, bool var_val);

    //Задание идентификатора контроллера в сети FabNet
    void set_ctrl_id(unsigned int ctrl_dev_id);
};

#endif