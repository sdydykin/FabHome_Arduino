#include <inttypes.h>

#include <Ds1302.h>

#include <FabNet_Arduino.h>

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

//Идентификатор Arduino-совместимого контроллера умного дома
#define CTRL_DEV_ID           0x0036

//Идентификатор двухканального блока реле
#define RELAY_BOARD_DEV_ID    0x002F

//Сеть FabNet
FabNet_Arduino fab_net(RS485_REn_PIN, RS485_DE_PIN, RS485_DI_PIN, RS485_RO_PIN, LED_NET_PIN);
//RTC модуль
Ds1302 rtc(RTC_CE_PIN, RTC_CLK_PIN, RTC_DAT_PIN);

//Начальная инициализация переферии
void setup() 
{
  //Старт пользовательского последовательного порта
  Serial.begin(9600);

  //Старт сети FabNet
  fab_net.begin();
  fab_net.set_ctrl_id(CTRL_DEV_ID);

  //Старт RTC модуля
  rtc.init();

  //Настройка светодиода индикации питания
  pinMode(LED_PWR_PIN, OUTPUT);

  //Включаем светодиод индикации питания
  digitalWrite(LED_PWR_PIN, HIGH);

  //Вывод приветствия
  Serial.println("**********************");
  Serial.println("***Relay board DEMO***");
  Serial.println("**********************");
}

//Главное меню 
#define RELAY1_ON       '1'
#define RELAY2_ON       '2'
#define RELAY1_OFF      '3'
#define RELAY2_OFF      '4'
#define RELAY1_STATE    '5'
#define RELAY2_STATE    '6'

void main_menu(void)
{
  Serial.println("");
  Serial.println("***Relay demo menu***");
  Serial.println("1 - Realy 1 ON");
  Serial.println("2 - Realy 2 ON");
  Serial.println("3 - Realy 1 OFF");
  Serial.println("4 - Realy 2 OFF");
  Serial.println("5 - Realy 1 state");
  Serial.println("6 - Realy 2 state");
}

//Меню часов реального времени
#define RTC_SET_TIME    '7'
#define RTC_GET_TIME    '8'

void rtc_menu(void)
{
  Serial.println("");
  Serial.println("***RTC demo menu***");
  Serial.println("7 - Set date/time");
  Serial.println("8 - Get current date/time");
}

//Приветствие к вводу команды:
void print_prompt(void)
{
  Serial.println("");
  Serial.println("Put command: ");
}

//Задание текущей даты и времени
void rtc1302_set_timestamp(void)
{
  //Задаваемое дата/время
  Ds1302::DateTime current_date_time;

  //Формируем задаваемое дата и время
  current_date_time.year = 24;
  current_date_time.month = 7;
  current_date_time.day = 1;
  current_date_time.dow = 1;
  current_date_time.hour = 14;
  current_date_time.minute = 0;
  current_date_time.second = 0;

  //Задаем текущее дата и время
  rtc.setDateTime(&current_date_time);

  //Выводим статус задания даты/времени
  Serial.println("Current date/time is UPDATED");
}

//Вывод текущей даты и времени
void rtc1302_get_timestamp(void)
{
  //Получаем текущу дату и время
  Ds1302::DateTime current_date_time;

  rtc.getDateTime(&current_date_time);

  //Преобразуем дату/время в строку
  char date_time_buf[20];
  sprintf(date_time_buf, "20%02d-%02d-%02d %02d:%02d:%02d", current_date_time.year, 
                                                            current_date_time.month, 
                                                            current_date_time.day, 
                                                            current_date_time.hour, 
                                                            current_date_time.minute, 
                                                            current_date_time.second);

  //Выводим дату/время
  Serial.print("Current date/time is ");
  Serial.println(date_time_buf);
}

//Основной цикл программы
void loop() 
{  
 
  //Обработчик команд меню
  main_menu();
  rtc_menu();
  print_prompt();

  while(!Serial.available());

  switch(Serial.read())
  {
    case RELAY1_ON:
      if(fab_net.write_bool_var(RELAY_BOARD_DEV_ID, RELAY_BOARD_CHANNEL1_VAR_ID, RELAY_BOARD_CHANNEL_ON))
        Serial.println("Relay 1 switched ON");
      else
        Serial.println("Relay 1 switched FAIL");

      break;
    case RELAY2_ON:
      if(fab_net.write_bool_var(RELAY_BOARD_DEV_ID, RELAY_BOARD_CHANNEL2_VAR_ID, RELAY_BOARD_CHANNEL_ON))
        Serial.println("Relay 2 switched ON");
      else
        Serial.println("Relay 2 switched FAIL");

      break;
    case RELAY1_OFF:
      if(fab_net.write_bool_var(RELAY_BOARD_DEV_ID, RELAY_BOARD_CHANNEL1_VAR_ID, RELAY_BOARD_CHANNEL_OFF))
        Serial.println("Relay 1 switched OFF");
      else
        Serial.println("Relay 1 switched FAIL");
      
      break;
    case RELAY2_OFF:
      if(fab_net.write_bool_var(RELAY_BOARD_DEV_ID, RELAY_BOARD_CHANNEL2_VAR_ID, RELAY_BOARD_CHANNEL_OFF)) 
        Serial.println("Relay 2 switched OFF");
      else
        Serial.println("Relay 2 switched FAIL");

      break;
    case RELAY1_STATE:
      bool relay1_state;

      if(fab_net.read_bool_var(RELAY_BOARD_DEV_ID, RELAY_BOARD_CHANNEL1_VAR_ID, &relay1_state))
      {  
        if(relay1_state)
          Serial.println("Relay 1 state is ON");
        else
          Serial.println("Relay 1 state is OFF");
      }
      else
        Serial.println("Can't read relay 1 state");

      break;
    case RELAY2_STATE:
      bool relay2_state;

      if(fab_net.read_bool_var(RELAY_BOARD_DEV_ID, RELAY_BOARD_CHANNEL2_VAR_ID, &relay2_state))
      {  
        if(relay2_state)
          Serial.println("Relay 2 state is ON");
        else
          Serial.println("Relay 2 state is OFF");
      }
      else
        Serial.println("Can't read relay 2 state");

      break;
    case RTC_SET_TIME:
      rtc1302_set_timestamp();

      break;
    case RTC_GET_TIME:
      rtc1302_get_timestamp();

      break;
    default: 
        Serial.println("Unknown command");
  }
}
