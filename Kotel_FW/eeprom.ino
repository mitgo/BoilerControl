#include <EEPROM.h>
#define START_EEPROM 1

const int eepromsize = sizeof(cfg_STAen)+sizeof(cfg_ssid)+sizeof(cfg_pass)+sizeof(cfg_lastGetIp)+sizeof(cfg_lastGetMask)+sizeof(cfg_lastGetGW)+sizeof(cfg_tries)+sizeof(cfg_timezone)+sizeof(cfg_temp_comf)+sizeof(cfg_temp_night)+sizeof(cfg_temp_min)+sizeof(cfg_dt)+sizeof(cfg_circ_hyst)+sizeof(cfg_ten_hyst)+sizeof(cfg_t_opt_pod)+sizeof(cfg_t_min_pod)+sizeof(cfg_t_min_obr)+sizeof(cfg_t_hot)+sizeof(cfg_time_morning)+sizeof(cfg_time_night)+sizeof(cfg_bheats)+sizeof(cfg_eheats)+sizeof(cfg_webuser)+sizeof(cfg_webpass)+sizeof(cfg_websessionperiod)+sizeof(cfg_mhost)+sizeof(cfg_mport)+sizeof(cfg_mpass)+sizeof(cfg_mqtt_server )+sizeof(cfg_mqttport)+sizeof(cfg_mqttperiod)+sizeof(cfg_ntpServerName)+sizeof(cfg_userMqtt)+sizeof(cfg_passMqtt)+4*8+1; //для хранения адресов датчиков температуры

void eepromInit() {
  EEPROM.begin(eepromsize);
    byte cfgver = 0;
  EEPROM.get(0,cfgver);
  EEPROM.end();
  if (cfgver != 1) {   // первый запуск
    EEPROM.begin(eepromsize);
    EEPROM.put(0, 1);
    delay(100);
    EEPROM.commit();
    resetConfig();
  }
  loadConfig();
}

void resetConfig() {
  dspLogOut("Pесет конфига");
  cfg_STAen = true;           //Подключаемся к роутеру (1) или делаем точку (0)
  strncpy(cfg_ssid, defSsid, sizeof(cfg_ssid));
  strncpy(cfg_pass, defPasswd, sizeof(cfg_pass));
  cfg_tries = 15;             //Сколько ожидать подключения
  strncpy(cfg_webuser, "admin", sizeof(cfg_webuser));
  strncpy(cfg_webpass, "admin", sizeof(cfg_webpass));
  cfg_websessionperiod = 300;
  strncpy(cfg_mqtt_server, "192.168.1.2", sizeof(cfg_mqtt_server));
  cfg_mqttport = 1883;
  cfg_mqttperiod = 30;
  cfg_timezone = 9;
  saveConfig();
}

bool saveConfig() {
  EEPROM.begin(eepromsize);
  int addr = START_EEPROM;
  delay(500);
  EEPROM.put(addr, cfg_STAen);
  EEPROM.put(addr += sizeof(cfg_STAen), cfg_ssid);
  EEPROM.put(addr += sizeof(cfg_ssid), cfg_pass); 
  EEPROM.put(addr += sizeof(cfg_pass), cfg_lastGetIp); 
  EEPROM.put(addr += sizeof(cfg_lastGetIp), cfg_lastGetMask); 
  EEPROM.put(addr += sizeof(cfg_lastGetMask), cfg_lastGetGW); 
  EEPROM.put(addr += sizeof(cfg_lastGetGW), cfg_tries);  
  EEPROM.put(addr += sizeof(cfg_tries), cfg_timezone);  
  EEPROM.put(addr += sizeof(cfg_timezone), cfg_temp_comf);
  EEPROM.put(addr += sizeof(cfg_temp_comf), cfg_temp_night);
  EEPROM.put(addr += sizeof(cfg_temp_night), cfg_temp_min);
  EEPROM.put(addr += sizeof(cfg_temp_min), cfg_dt);
  EEPROM.put(addr += sizeof(cfg_dt), cfg_circ_hyst);
  EEPROM.put(addr += sizeof(cfg_circ_hyst), cfg_ten_hyst);
  EEPROM.put(addr += sizeof(cfg_ten_hyst), cfg_t_opt_pod);
  EEPROM.put(addr += sizeof(cfg_t_opt_pod), cfg_t_min_pod);
  EEPROM.put(addr += sizeof(cfg_t_min_pod), cfg_t_min_obr);
  EEPROM.put(addr += sizeof(cfg_t_min_obr), cfg_t_hot);
  EEPROM.put(addr += sizeof(cfg_t_hot), cfg_time_morning);
  EEPROM.put(addr += sizeof(cfg_time_morning), cfg_time_night);
  EEPROM.put(addr += sizeof(cfg_time_night), cfg_bheats);  
  EEPROM.put(addr += sizeof(cfg_bheats), cfg_eheats);  
  EEPROM.put(addr += sizeof(cfg_eheats), cfg_webuser);  
  EEPROM.put(addr += sizeof(cfg_webuser), cfg_webpass);  
  EEPROM.put(addr += sizeof(cfg_webpass), cfg_websessionperiod); 
  EEPROM.put(addr += sizeof(cfg_websessionperiod), cfg_mqtt_server);  
  EEPROM.put(addr += sizeof(cfg_mqtt_server), cfg_mqttport);  
  EEPROM.put(addr += sizeof(cfg_mqttport), cfg_userMqtt);
  EEPROM.put(addr += sizeof(cfg_userMqtt), cfg_passMqtt);   
  EEPROM.put(addr += sizeof(cfg_passMqtt), cfg_mqttperiod);  
  EEPROM.put(addr += sizeof(cfg_mqttperiod), cfg_mhost);  
  EEPROM.put(addr += sizeof(cfg_mhost), cfg_mport);  
  EEPROM.put(addr += sizeof(cfg_mport),cfg_ntpServerName);
  addr += sizeof(cfg_ntpServerName);
  for (byte i = 0; i < 6; i++)
    EEPROM.put(addr + i, cfg_mpass[i]);
  addr += 6;  
  for (byte i = 0; i < 8; i++)
    EEPROM.put(addr + i, cfg_addrIn[i]);
  addr += 8;  
  for (byte i = 0; i < 8; i++)
    EEPROM.put(addr + i, cfg_addrOut[i]);
  addr += 8;
  for (byte i = 0; i < 8; i++)
    EEPROM.put(addr + i, cfg_addrPodacha[i]);
  addr += 8;
  for (byte i = 0; i < 8; i++)
    EEPROM.put(addr + i, cfg_addrObratka[i]);

  bool a=EEPROM.commit();
  EEPROM.end();
  DBG_PRN("Config Saved");
  dspLogOut("Конфиг сохранен");
  if (a) {
    DBG_PRN("Config Saved");
    dspLogOut("Конфиг сохранен");
    return true;
  }
  else {
    DBG_PRN("Config NOT Saved");
    dspLogOut("Конфиг не сохранен");    
    return false;  
  }
}

void loadConfig() {
  EEPROM.begin(eepromsize);
  int addr = START_EEPROM;
  delay(500);
  EEPROM.get(addr, cfg_STAen);
  EEPROM.get(addr += sizeof(cfg_STAen), cfg_ssid);
  EEPROM.get(addr += sizeof(cfg_ssid), cfg_pass);
  EEPROM.get(addr += sizeof(cfg_pass), cfg_lastGetIp); 
  EEPROM.get(addr += sizeof(cfg_lastGetIp), cfg_lastGetMask); 
  EEPROM.get(addr += sizeof(cfg_lastGetMask), cfg_lastGetGW);     
  EEPROM.get(addr += sizeof(cfg_lastGetGW), cfg_tries);  
  EEPROM.get(addr += sizeof(cfg_tries), cfg_timezone);  
  EEPROM.get(addr += sizeof(cfg_timezone), cfg_temp_comf);
  EEPROM.get(addr += sizeof(cfg_temp_comf), cfg_temp_night);
  EEPROM.get(addr += sizeof(cfg_temp_night), cfg_temp_min);
  EEPROM.get(addr += sizeof(cfg_temp_min), cfg_dt);
  EEPROM.get(addr += sizeof(cfg_dt), cfg_circ_hyst);
  EEPROM.get(addr += sizeof(cfg_circ_hyst), cfg_ten_hyst);
  EEPROM.get(addr += sizeof(cfg_ten_hyst), cfg_t_opt_pod);
  EEPROM.get(addr += sizeof(cfg_t_opt_pod), cfg_t_min_pod);
  EEPROM.get(addr += sizeof(cfg_t_min_pod), cfg_t_min_obr);
  EEPROM.get(addr += sizeof(cfg_t_min_obr), cfg_t_hot);
  EEPROM.get(addr += sizeof(cfg_t_hot), cfg_time_morning);
  EEPROM.get(addr += sizeof(cfg_time_morning), cfg_time_night);
  EEPROM.get(addr += sizeof(cfg_time_night), cfg_bheats);  
  EEPROM.get(addr += sizeof(cfg_bheats), cfg_eheats);  
  EEPROM.get(addr += sizeof(cfg_time_night), cfg_webuser);  
  EEPROM.get(addr += sizeof(cfg_webuser), cfg_webpass);  
  EEPROM.get(addr += sizeof(cfg_webpass), cfg_websessionperiod); 
  EEPROM.get(addr += sizeof(cfg_websessionperiod), cfg_mqtt_server);  
  EEPROM.get(addr += sizeof(cfg_mqtt_server), cfg_mqttport);  
  EEPROM.get(addr += sizeof(cfg_mqttport), cfg_userMqtt);
  EEPROM.get(addr += sizeof(cfg_userMqtt), cfg_passMqtt);   
  EEPROM.get(addr += sizeof(cfg_passMqtt), cfg_mqttperiod);  
  EEPROM.get(addr += sizeof(cfg_mqttperiod), cfg_mhost);
  EEPROM.get(addr += sizeof(cfg_mhost), cfg_mport);
  EEPROM.get(addr += sizeof(cfg_mport),cfg_ntpServerName);
  addr+=sizeof(cfg_ntpServerName);
  for (byte i = 0; i < 6; i++)
    EEPROM.get(addr + i, cfg_mpass[i]);
  addr += 6; 
  for (byte i = 0; i < 8; i++)
    EEPROM.get(addr + i, cfg_addrIn[i]);
  addr += 8;  
  for (byte i = 0; i < 8; i++)
    EEPROM.get(addr+i, cfg_addrOut[i]);
  addr += 8;
  for (byte i = 0; i < 8; i++)
    EEPROM.get(addr + i, cfg_addrPodacha[i]);
  addr += 8;
  for (byte i = 0; i < 8; i++)
    EEPROM.get(addr + i, cfg_addrObratka[i]);
  DBG_PRN("Config Loaded");
  dspLogOut("Конфиг загружен");
  EEPROM.end();
}
void saveIpSettings() {
  EEPROM.begin(eepromsize);
  int addr = START_EEPROM + sizeof(cfg_STAen)+sizeof(cfg_ssid)+sizeof(cfg_pass);
  delay(500);
  EEPROM.put(addr, cfg_lastGetIp);
  EEPROM.put(addr += sizeof(cfg_lastGetIp), cfg_lastGetMask); 
  EEPROM.put(addr += sizeof(cfg_lastGetMask), cfg_lastGetGW); 
  EEPROM.commit();
  EEPROM.end();
}
