#include "FabNet_Arduino.h"

//Конструктор класса
FabNet_Arduino::FabNet_Arduino(SoftwareSerial& serial, unsigned char re_pin, unsigned char de_pin) {
  //Сохраняем выводы управления драйвером сети FabNet
  _re_pin = re_pin; 
  _de_pin = de_pin;

  //Сохраняем последовательный порт для доступа к сети FabNet
  _serial = serial;
}
    
//Настройка аппаратной конфигурации
void FabNet_Arduino::begin(void) {
  //Настраиваем пины управления драйвером сети FabNet
  pinMode(_re_pin, OUTPUT);
  pinMode(_de_pin, OUTPUT);

  //Переводим драйвер в режим приема
  digitalWrite(_re_pin, LOW);
  digitalWrite(_de_pin, LOW);
} 
    
//Чтение вещественной переменной
bool FabNet_Arduino::read_float_var(unsigned int dev_id, unsigned char var_id, float * var_val) {
  return true;
}

//Чтение целочисленной переменной со знаком
bool FabNet_Arduino::read_int_var(unsigned int dev_id, unsigned char var_id, long * var_val) {
  return true;
}

//Чтение целочисленной перемпенной без знака
bool FabNet_Arduino::read_uint_var(unsigned int dev_id, unsigned char var_id, unsigned long * var_val) {
  return true;
}

//Чтение логической переменной
bool FabNet_Arduino::read_bool_var(unsigned int dev_id, unsigned char var_id, bool * var_val) {
  return true;
}

//Запись логической переменной
bool FabNet_Arduino::write_bool_var(unsigned int dev_id, unsigned char var_id, bool var_val) {
  return true;
}

//Задание идентификатора контроллера в сети FabNet
void FabNet_Arduino::set_ctrl_id(unsigned int ctrl_dev_id)
{
  //Сохраняем идентификатор контроллера в сети FabNet
  _ctrl_dev_id = ctrl_dev_id;
}