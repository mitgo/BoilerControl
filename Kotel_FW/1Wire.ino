#include <OneWire.h>                //Датчики температуры
#if (ONWORK >= 1)
#define DS      10  //контакт датчиков температуры
#else
#define DS      3  //контакт датчиков температуры
#endif
#define kol_DS  4  //количество датчиков температуры
OneWire ds(DS); 


void tempSensor::begin(byte *addr, int freeq) { // Конструктор      
  for (int i = 0; i < 8; i++)
    tempSensor::_addr[i] = addr[i]; 
  tempSensor::_freeqScan = freeq;
  tempSensor::_curr = 0;
  for (int i=0; i++; i < 5)
    tempSensor::_temparr[i] = 0;    
  tempSensor::conn = false;
  tempSensor::_lastScan = millis();
  tempSensor::_lastSuccessGet = millis();
  tempSensor::_readReq = false;
  tempSensor::searchDS();
};

unsigned long tempSensor::lastSucGet() {
  return tempSensor::_lastSuccessGet;
}

String tempSensor::addrStr() {
  String addr = "";
  if (tempSensor::conn) {
    for (int i = 0; i < 8; i++) {
      addr += (tempSensor::_addr[i] < 0x10) ? "0": "";
      addr += String(tempSensor::_addr[i], HEX); 
      if (i < 7) addr += ":";
      }  
  }
  else addr = "No sensor on BUS";
  return addr;  
};

void tempSensor::prepare() {
    ds.reset();
    ds.select(tempSensor::_addr);
    ds.write(0x44, 1); // start conversion, with parasite power on at the end
    /*После окончания преобразования данные сохраняются в 2-байтовом температурном регистре в
       оперативной памяти, а DS18B20 возвращается в неактивное состояние с низким
      энергопотреблением*/
}

float tempSensor::getTemp() {// Получение температуры от датчика
  if (tempSensor::conn) 
  {  
    if (!tempSensor::_readReq) {  
      tempSensor::_readReq = !tempSensor::_readReq;
      tempSensor::_lastScan = millis();
      tempSensor::prepare();
    }
    else if ((millis()-tempSensor::_lastScan)/1000 >= tempSensor::_freeqScan) { 
      byte data[9];//массив: строки-ном датчика по 12 байт с температурой
      ds.reset();//работа с датчиком всегда должна начинаться с сигнала ресета
      ds.select(_addr);//выбирается адрес (ROM ) датчика
      //Чтобы проверить корректность записи данных, необходимо выполнить чтение (используя команду чтения
      //Read Scratchpad [BEh]) после того, как данные (какие) будут записаны(куда).
      ds.write(0xBE);
      /*(0xBE = 190)/читать содержание ПАМЯТИ. Передача данных начинается с наименьшего значащего бита байта 0 и продолжается до 9-ого байта (байт 8
         - циклический контроль избыточности). */
      for ( byte i = 0; i < 9; i++)
        data[i] = ds.read();//сохраняем температуру(9 байт) в массиве соответствующего датчика
      
      //проверка контрольной суммы:Старишие 8бит из 64 ROM, а не температуры!! содержат crc
      if (OneWire::crc8(data, 8) != data[8]) {
        DBG_PRN("Data CRC is not valid!");
      }
      else {
        int16_t raw = (data[1] << 8) | data[0];//Тип 16-разрядных целых
        byte cfg = (data[4] & 0x60);
        if (cfg == 0x00) raw = raw & ~7; // 9 bit resolution, 93.75 ms
        else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
        else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
        int tmp = round((float)raw / 16.0 * 10.0); 
         // Скачек температуры должен быть не больше 5 градусов, чтобы отловить отвал датчика с 30 градусов до 85
         // 85 градусов - полуотвал датчика, а так же 127,9 градусов
        if ((tmp != 850 and tmp != 1279) or (abs(tmp - tempSensor::_temparr[tempSensor::_curr]) < 50)){
          tempSensor::_curr++;
          if (tempSensor::_curr >= 6)
            tempSensor::_curr = 0;
          tempSensor::_temparr[_curr] = tmp; //округляем температуру в целых
          tempSensor::_readReq = !tempSensor::_readReq;
          tempSensor::_lastSuccessGet = millis();
        }
        else {
          tempSensor::conn = false;
        }
      }  
    }  
    return (float)tempSensor::_temparr[tempSensor::_curr] / 10;
  }
  else  {
    DBG_PRN("No sensor or Bad sensor");
    return -2000;
  }
};

float tempSensor::prevTemp() {
  return (float)tempSensor::_temparr[tempSensor::_curr - 1] / 10;
};

void tempSensor::searchDS(){
  ds.reset_search();
  byte addr[8];// массив: строки-кол-во датчиков по 8 байт в каждой для хранения   
  byte i;
  while (ds.search(addr)) {
    if (OneWire::crc8(addr, 7) == addr[7]) {
      for (i = 0; i < 8; i++) {
        if (tempSensor::_addr[i] != addr[i]) {
          tempSensor::conn = false;
          break;
        }
      }
      if (i == 8) {
        tempSensor::conn = true; 
        return;  
      }
    } 
    tempSensor::conn = false;
  }
  tempSensor::conn = false;  
  return;
};

String getUnConfSensors() { 
  String AllData = "";
  ds.reset_search();
  byte num = 0;
  byte j = 0;
  byte addr[8];// временный массив адреса каждого датчика  
  while ( ds.search(addr)) {
    if (OneWire::crc8(addr, 7) == addr[7] and addr[0]==0x28) {
        for (j = 1; j < 8; j++) {
          if ( (addr[j] != cfg_addrIn[j]) and (addr[j] != cfg_addrOut[j]) and (addr[j] != cfg_addrPodacha[j]) and (addr[j] != cfg_addrObratka[j]) )
            break;
        }
       // Если диагностируемый адрес не совпал ни с одним из конфига
        if (j!=8) { 
            num++;
            AllData += String(num) + ". ";
            for (byte i = 0; i < 8; i++) {
              AllData += (addr[i] < 0x10) ? "0": "";
              AllData += String(addr[i], HEX);
              if (i < 7) AllData += ":"; else AllData += " Temp: ";
            }  
            byte data[9];
            delay(250);
            //Готовим датчик к считыванию температуры
            ds.reset();
            ds.select(addr);
            // Комманда на чтение температуры.
            ds.write(0x44, 1);
            delay(750);
            //Читаем температуру
            ds.reset();
            ds.select(addr);
            ds.write(0xBE);            
            for ( byte i = 0; i < 9; i++)
              data[i] = ds.read();//сохраняем температуру(9 байт) в массиве соответствующего датчика
            if (OneWire::crc8(data, 8) != data[8]) {
              AllData += " Bad Data";
            }  
            int16_t raw = (data[1] << 8) | data[0];//Тип 16-разрядных целых
            byte cfg = (data[4] & 0x60);
            if (cfg == 0x00) raw = raw & ~7; // 9 bit resolution, 93.75 ms
            else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
            else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
            AllData += " " + String((float)raw / 16.0) + "<br />"; //округляем температуру в целых
        }    
    } 
    else AllData += "Bad Addr CRC<br />"; 
  }
  return AllData;
};  
