#include "FabNet_Arduino.h"

//Расчет контрольной суммы пакета
uint8_t FabNet_Arduino::rs485_calc_crc(uint8_t * data, uint8_t size) 
{
	//Расчитанное значение CRC
	uint8_t crc = 0xFF;

	//Счетчик бит очередного обрабатываемого байта
  uint8_t i;

  while(size--)
  {
    crc ^= *data++;

    for(i = 0; i < 8; i++)

    crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
  }

  return crc;
}

//Прием данных из сети RS485
bool FabNet_Arduino::rs485_recv(void * data, uint8_t size) 
{
  //Буфер для принимаемого пакета
  uint8_t pkg_buf[RS485_PKG_SIZE];  

  //Размер принятого пакета
  uint8_t recv_pkg_size;

  //Количество обработанных байт в пакете
  uint8_t recv_pkg_handled = 0;

  //Количество скопированных пользователю байт
  uint8_t copy_data_count = 0;

  //Указатель на принятые данные
  uint8_t * data_ptr = data;

  //Пытаемся принять пакет
  recv_pkg_size = _serial.readBytes(pkg_buf, RS485_PKG_SIZE);

  if(recv_pkg_size > 0)
  {
    //Разбираем принятый пакет

    //Поиск начала пакета
    while(pkg_buf[recv_pkg_handled] != RS485_PHY_SOP)
    {
      recv_pkg_handled++;

      if(recv_pkg_handled == recv_pkg_size)
        return false;
    }

    recv_pkg_handled++;

    //Удаление экранирующих символов
    while(recv_pkg_handled < (recv_pkg_size - 1))
    {
      if(pkg_buf[recv_pkg_handled] == RS485_PHY_ESCAPE)
      {
        recv_pkg_handled++;                    
      }

      data_ptr[copy_data_count] = pkg_buf[recv_pkg_handled];

      recv_pkg_handled++;                    

      copy_data_count++;
    }

    //Проверка наичия признака конца пакета
    if(pkg_buf[recv_pkg_handled] != RS485_PHY_EOP)
      return false;

    //Проверка контрольной суммы
    if(data_ptr[copy_data_count - 1] != rs485_calc_crc(data_ptr, copy_data_count - 1))
    {
      return false;
    }

    return true;
  }

  return false;
}

//Передача данных в сеть RS485
bool FabNet_Arduino::rs485_send(void * data, uint8_t size) 
{
  //Буфер для передаваемого пакета
  uint8_t pkg_buf[RS485_PKG_SIZE];  
  uint8_t pkg_buf_size = 0;
  uint8_t pkg_buf_send = 0;

  //Указатель на передаваемые данные
  uint8_t * data_ptr = data;

  //Переводим драйвер в режим передачи
  digitalWrite(_re_pin, HIGH);
  digitalWrite(_de_pin, HIGH);

  //Формируем передаваемый пакет

  //Признак начала пакета
  pkg_buf[pkg_buf_size] = RS485_PHY_SOP;
  pkg_buf_size++;

  //Добавляем экранирующие символы в пакет
  for(uint8_t i = 0; i < (size - 1); i++)
  {
    if(data_ptr[i] == RS485_PHY_ESCAPE || data_ptr[i] == RS485_PHY_SOP || data_ptr[i] == RS485_PHY_EOP)
    {    
      pkg_buf[pkg_buf_size] = RS485_PHY_ESCAPE; 
      pkg_buf_size++;
    }

    pkg_buf[pkg_buf_size] = data_ptr[i]; 
    pkg_buf_size++;
  }

  //Добавляем контрольную сумму
  pkg_buf[pkg_buf_size] = rs485_calc_crc(data, size - 1);
  pkg_buf_size++;

  //Добавляем признак конца пакета
  pkg_buf[pkg_buf_size] = RS485_PHY_EOP;
  pkg_buf_size++;

  //Отправляем пакет
  pkg_buf_send = _serial.write(pkg_buf, pkg_buf_size);
	
  //Ждем завершения отправки
  _serial.flush();  

  //Переводим драйвер в режим приема
  digitalWrite(_re_pin, LOW);
  digitalWrite(_de_pin, LOW);

  //Проверяем все ли данные были отправлены
  if(pkg_buf_send == pkg_buf_size)
    return true;

	return false;
}

//Мигание светодиодом активности сети
void FabNet_Arduino::net_led_blink(void)
{
  digitalWrite(_led_pin, HIGH);
  delay(50);
  digitalWrite(_led_pin, LOW);
}

//Формирования пакета запроса чтения переменной
void FabNet_Arduino::create_get_var_pkg(t_rs485_pkg * rs485_pkg, uint16_t dev_id, uint8_t var_id)
{
  //Формируем запрос чтения переменной
  rs485_pkg->hdr.pkg_type = PKG_RS485_GET_VAR_ID;
  rs485_pkg->hdr.pkg_size = PKG_RS485_GET_VAR_SIZE;

  rs485_pkg->data.get_var_pkg.id = var_id;

  //Заполняем общие поля пакета
  rs485_pkg->hdr.src_id = _ctrl_dev_id;
  rs485_pkg->hdr.dst_id = dev_id;
}

//Формирования пакета запроса записи переменной
void FabNet_Arduino::create_set_var_pkg(t_rs485_pkg * rs485_pkg, uint16_t dev_id, uint8_t var_id, uint8_t var_type, t_var_value var_val)
{
  //Формируем запрос записи переменной
  rs485_pkg->hdr.pkg_type = PKG_RS485_SET_VAR_ID;
  rs485_pkg->hdr.pkg_size = PKG_RS485_SET_VAR_SIZE;

  rs485_pkg->data.set_var_pkg.id = var_id;
  rs485_pkg->data.set_var_pkg.type = var_type;
  rs485_pkg->data.set_var_pkg.value = var_val;

  //Заполняем общие поля пакета
  rs485_pkg->hdr.src_id = _ctrl_dev_id;
  rs485_pkg->hdr.dst_id = dev_id;
}

//Разбор oтвета на запрос чтения переменной
bool FabNet_Arduino::parse_var_ack_pkg(t_rs485_pkg * rs485_pkg, uint16_t dev_id, uint8_t var_id, uint8_t vat_type, t_var_value * var_val)
{		
  //Проверяем маршрутную информацию
	if((rs485_pkg->hdr.src_id != dev_id) || (rs485_pkg->hdr.dst_id != _ctrl_dev_id))
    return false;
			
	//Разбираем ответ на запрос чтения переменной
	if((rs485_pkg->hdr.pkg_type == PKG_RS485_VAR_ACK_ID) && (rs485_pkg->data.var_ack_pkg.id == var_id) && (rs485_pkg->data.var_ack_pkg.type == vat_type))
  {
    *var_val = rs485_pkg->data.var_ack_pkg.value;

    return true;
  }

  return false;
}

//Разбор ответного пакета
bool FabNet_Arduino::parse_ack_pkg(t_rs485_pkg * rs485_pkg, uint16_t dev_id)
{
  //Проверяем маршрутную информацию
	if((rs485_pkg->hdr.src_id != dev_id) || (rs485_pkg->hdr.dst_id != _ctrl_dev_id))
      return false;
			
	//Разбираем ответный пакет
	if((rs485_pkg->hdr.pkg_type == PKG_RS485_ACK_ID) && (rs485_pkg->data.ack_pkg.status == PKG_RS485_ACK_OK))
    return true;
  
  return false;
}

//Конструктор класса
FabNet_Arduino::FabNet_Arduino(uint8_t re_pin, uint8_t de_pin, uint8_t ro_pin, uint8_t di_pin, uint8_t led_pin)  : _serial(di_pin, ro_pin)
{
  //Сохраняем выводы управления драйвером сети FabNet
  _re_pin = re_pin; 
  _de_pin = de_pin;

  //Сохраняем вывод управления светодиодом индикации сетевой активности
  _led_pin = led_pin;
}
    
//Настройка аппаратной конфигурации
void FabNet_Arduino::begin(void) 
{
  //Старт последовательного интерфейса для доступа к сети FabNet
  _serial.begin(19200);
  _serial.setTimeout(50);

  //Настраиваем пины управления драйвером сети FabNet
  pinMode(_re_pin, OUTPUT);
  pinMode(_de_pin, OUTPUT);

  //Переводим драйвер в режим приема
  digitalWrite(_re_pin, LOW);
  digitalWrite(_de_pin, LOW);

  //Настраиваем пин управлениясветодиодом активности сети
  pinMode(_led_pin, OUTPUT);
} 
    
//Чтение вещественной переменной
bool FabNet_Arduino::read_float_var(uint16_t dev_id, uint8_t var_id, float * var_val) 
{
  //Буфер передаваемого пакета
  t_rs485_pkg net_tx_buf;
  //Буфер принимаемого пакета
  t_rs485_pkg net_rx_buf;
  //Значение переменной
  t_var_value read_var;

  //Формируем запрос чтения переменной
  create_get_var_pkg(&net_tx_buf, dev_id, var_id);

  //Отправляем пакет
  rs485_send(&net_tx_buf, net_tx_buf.hdr.pkg_size);

  //Принимаем ответ
  if(rs485_recv(&net_rx_buf, sizeof(net_rx_buf)))
	{
		//Разбираем ответ на запрос чтения переменной
    if(parse_var_ack_pkg(&net_rx_buf, dev_id, var_id, VAR_TYPE_REAL, &read_var))
    {
      *var_val = read_var.real;
      
      net_led_blink();
      
      return true;
    }
  }

  return false;
}

//Чтение целочисленной переменной со знаком
bool FabNet_Arduino::read_int_var(uint16_t dev_id, uint8_t var_id, int32_t * var_val) 
{
  //Буфер передаваемого пакета
  t_rs485_pkg net_tx_buf;
  //Буфер принимаемого пакета
  t_rs485_pkg net_rx_buf;
  //Значение переменной
  t_var_value read_var;

  //Формируем запрос чтения переменной
  create_get_var_pkg(&net_tx_buf, dev_id, var_id);

  //Отправляем пакет
  rs485_send(&net_tx_buf, net_tx_buf.hdr.pkg_size);

  //Принимаем ответ
  if(rs485_recv(&net_rx_buf, sizeof(net_rx_buf)))
	{
		//Разбираем ответ на запрос чтения переменной
    if(parse_var_ack_pkg(&net_rx_buf, dev_id, var_id, VAR_TYPE_SIGNED, &read_var))
    {
      *var_val = read_var.signed_int;
      
      net_led_blink();
      
      return true;
    }
  }

  return false;
}

//Чтение целочисленной перемпенной без знака
bool FabNet_Arduino::read_uint_var(uint16_t dev_id, uint8_t var_id, uint32_t * var_val) 
{
  //Буфер передаваемого пакета
  t_rs485_pkg net_tx_buf;
  //Буфер принимаемого пакета
  t_rs485_pkg net_rx_buf;
  //Значение переменной
  t_var_value read_var;

  //Формируем запрос чтения переменной
  create_get_var_pkg(&net_tx_buf, dev_id, var_id);

  //Отправляем пакет
  rs485_send(&net_tx_buf, net_tx_buf.hdr.pkg_size);

  //Принимаем ответ
  if(rs485_recv(&net_rx_buf, sizeof(net_rx_buf)))
	{
		//Разбираем ответ на запрос чтения переменной
    if(parse_var_ack_pkg(&net_rx_buf, dev_id, var_id, VAR_TYPE_UNSIGNED, &read_var))
    {
      *var_val = read_var.unsigned_int;
      
      net_led_blink();
      
      return true;
    }
  }

  return false;
}

//Чтение логической переменной
bool FabNet_Arduino::read_bool_var(unsigned int dev_id, uint8_t var_id, bool * var_val) 
{
  //Буфер передаваемого пакета
  t_rs485_pkg net_tx_buf;
  //Буфер принимаемого пакета
  t_rs485_pkg net_rx_buf;
  //Значение переменной
  t_var_value read_var;

  //Формируем запрос чтения переменной
  create_get_var_pkg(&net_tx_buf, dev_id, var_id);

  //Отправляем пакет
  rs485_send(&net_tx_buf, net_tx_buf.hdr.pkg_size);

  //Принимаем ответ
  if(rs485_recv(&net_rx_buf, sizeof(net_rx_buf)))
	{
		//Разбираем ответ на запрос чтения переменной
    if(parse_var_ack_pkg(&net_rx_buf, dev_id, var_id, VAR_TYPE_LOGIC, &read_var))
    {
      *var_val = read_var.logic;
      
      net_led_blink();
      
      return true;
    }
  }

  return false;
}

//Запись логической переменной
bool FabNet_Arduino::write_bool_var(unsigned int dev_id, unsigned char var_id, bool var_val) 
{
  //Буфер передаваемого пакета
  t_rs485_pkg net_tx_buf;
  //Буфер принимаемого пакета
  t_rs485_pkg net_rx_buf;
  //Значение переменной
  t_var_value write_var;

  //Формируем записываемое значение переменной
  write_var.logic = var_val; 

  //Формируем запрос записи переменной
  create_set_var_pkg(&net_tx_buf, dev_id, var_id, VAR_TYPE_LOGIC, write_var);
  
  //Отправляем пакет
  rs485_send(&net_tx_buf, net_tx_buf.hdr.pkg_size);

  //Принимаем ответ
  if(rs485_recv(&net_rx_buf, sizeof(net_rx_buf)))
	{	
		//Разбираем ответ на запрос записи переменной
		if(parse_ack_pkg(&net_rx_buf, dev_id))
    {  
      net_led_blink();

      return true;
    }
  }      

  return false;
}

//Задание идентификатора контроллера в сети FabNet
void FabNet_Arduino::set_ctrl_id(unsigned int ctrl_dev_id)
{
  //Сохраняем идентификатор контроллера в сети FabNet
  _ctrl_dev_id = ctrl_dev_id;
}
