// Web интерфейс для устройства HTTP_init
#include <ESP8266WebServer.h>       // Web сервер
ESP8266WebServer HTTP(80);

unsigned long ESPFreeSketchSpace = ESP.getFreeSketchSpace(); 

void HTTP_init(void) {
  HTTP.on("/", handle_Root);
  HTTP.on("/config", handle_Config);
  HTTP.on("/saveconf", handle_saveconf);
  HTTP.on("/update", HTTP_POST, [](){
      HTTP.sendHeader("Connection", "close");
      HTTP.sendHeader("Access-Control-Allow-Origin", "*");
      HTTP.send(200, "text/plain", String("<body onload='setInterval(function(){ window.history.go(-1)}, 8000);'> " + utf8rus("Обновление: ") + String((Update.hasError())?"FAIL":"OK") + "</body>").c_str());
      delay(500);
      ESP.restart();
    }, []() {     
      HTTPUpload& upload = HTTP.upload();
      if(upload.status == UPLOAD_FILE_START){
        //Serial.setDebugOutput(true);
        uint32_t maxSketchSpace = (ESPFreeSketchSpace - 0x1000) & 0xFFFFF000;
        if(!Update.begin(maxSketchSpace)){//start with max available size
         // Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_WRITE){
        if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
         // Update.printError(Serial);
        }
      } else if(upload.status == UPLOAD_FILE_END){
        if(Update.end(true)){ //true to set the size to the current progress
          //Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
        //  Update.printError(Serial);
        }
     //   Serial.setDebugOutput(false);
      }
      yield();
    });
  HTTP.on("/restart", handle_Restart);   // Перезагрузка модуля по запросу вида /restart?device=ok
  // Запускаем HTTP сервер
  const char * headerkeys[] = {"User-Agent", "Cookie"} ;
  size_t headerkeyssize = sizeof(headerkeys) / sizeof(char*);
  //ask server to track these headers
  HTTP.collectHeaders(headerkeys, headerkeyssize);
  HTTP.begin();
}


void HTTP_loop() {
  HTTP.handleClient();
}

// Функции API-Set
// Установка параметров для подключения к внешней AP по запросу вида http://192.168.0.101/ssid?ssid=home2&password=12345678
void handle_Set_Ssid() {
  strncpy(cfg_ssid, HTTP.arg("ssid").c_str(), sizeof(cfg_ssid));
  strncpy(cfg_pass, HTTP.arg("pass").c_str(), sizeof(cfg_pass));
  saveConfig();                        // Функция сохранения данных во Flash пока пустая
  HTTP.send(200, "text/plain", String("<body onload='setInterval(function(){ window.history.go(-1)}, 8000);'> " + utf8rus("Настройки для точки: ") + HTTP.arg("ssid") + utf8rus(" установлены") + "</body>").c_str());   // отправляем ответ о выполнении
}

// Перезагрузка модуля по запросу вида http://192.168.0.101/restart?device=ok
void handle_Restart() {
  String restart = HTTP.arg("device");          // Получаем значение device из запроса
  if (restart == "ok") {                         // Если значение равно Ок
    HTTP.send(200, "text / plain", String("<body onload='setInterval(function(){ window.history.go(-1)}, 10000);'> " + utf8rus("Модуль перезагружается, вы будете перенаправлены на главную страницу") + "</body>").c_str()); // Oтправляем ответ Reset OK
    ESP.restart();                                // перезагружаем модуль
  }
  else {                                        // иначе
    HTTP.send(200, "text / plain", utf8rus("Неверный параметр")); // Oтправляем ответ No Reset
  }
}
  

void handle_saveconf () {
//  cfg_STAen = HTTP.arg("STAen").toInt();
  strncpy(cfg_ssid, HTTP.arg("ssid").c_str(), sizeof(cfg_ssid));
  strncpy(cfg_pass, HTTP.arg("pass").c_str(), sizeof(cfg_pass));
//  cfg_tries = HTTP.arg("tries").toInt();
  strncpy(cfg_ntpServerName, HTTP.arg("ntp").c_str(), sizeof(cfg_ntpServerName));
  cfg_timezone = HTTP.arg("timezone").toInt();
  cfg_temp_comf = HTTP.arg("t_compf").toInt();
  cfg_temp_night = HTTP.arg("t_night").toInt();
  cfg_temp_min = HTTP.arg("temp_min").toInt();
  cfg_dt = HTTP.arg("dt").toInt();
  cfg_circ_hyst = HTTP.arg("circ_hyst").toInt();
  cfg_ten_hyst = HTTP.arg("ten_hyst").toInt();
  cfg_t_opt_pod = HTTP.arg("t_opt_pod").toInt();
  cfg_t_min_pod = HTTP.arg("t_min_pod").toInt();
  cfg_t_min_obr = HTTP.arg("t_min_obr").toInt();
  cfg_t_hot = HTTP.arg("t_hot").toInt();
  cfg_time_morning = HTTP.arg("morning").toInt();
  cfg_time_night = HTTP.arg("night").toInt();
  cfg_bheats = HTTP.arg("begheatseason").toInt();
  cfg_eheats = HTTP.arg("endheatseason").toInt();
  strncpy(cfg_webuser, HTTP.arg("webuser").c_str(), sizeof(cfg_webuser));
  strncpy(cfg_webpass, HTTP.arg("webpass").c_str(), sizeof(cfg_webpass));
  cfg_websessionperiod = HTTP.arg("websessionperiod").toInt();
//  cfg_mqttperiod = HTTP.arg("mqttperiod").toInt();
  strncpy(cfg_mqtt_server, HTTP.arg("mqtt_server").c_str(), sizeof(cfg_mqtt_server));
  cfg_mqttport = HTTP.arg("mqttport").toInt();
  strncpy(cfg_userMqtt, HTTP.arg("userMqtt").c_str(), sizeof(cfg_userMqtt));
  strncpy(cfg_passMqtt, HTTP.arg("passMqtt").c_str(), sizeof(cfg_passMqtt));
  strncpy(cfg_mhost, HTTP.arg("mhost").c_str(), sizeof(cfg_mhost));
  cfg_mport = HTTP.arg("mport").toInt();
  String _addr;
  _addr = HTTP.arg("mpass");
  for (int i = 0; i < 6; i++)
    cfg_mpass[i] = (int)_addr[i] - '0';
  byte _addr_arr[8];  
  char buf[16];
  String warn = "";
  String addresses[] = {"inaddr", "outaddr", "podachaaddr", "obratkaaddr"}; 
  for (byte i = 0; i < 4; i++) {
    _addr = HTTP.arg(addresses[i]);
    if (_addr != "No sensor on BUS") {
      _addr.replace(":", "");
      if (_addr.length() == 16 or _addr.length() == 0) {
        _addr.toCharArray(buf, 18);
        convert(buf, _addr_arr);
        byte j;
        for (j = 1; j < 8; j++) {
          if ( (_addr_arr[j] != cfg_addrIn[j] or i == 0) and (_addr_arr[j] != cfg_addrOut[j] or i == 1) and (_addr_arr[j] != cfg_addrPodacha[j] or i == 2) and (_addr_arr[j] != cfg_addrObratka[j] or i == 3) )
            break;
        }
        if (j != 8 or _addr.length() == 0)
          switch  (i) {
            case 0:
              convert(buf, cfg_addrIn);
              In.begin(cfg_addrIn, cfg_freq);
              break;
            case 1:
              convert(buf, cfg_addrOut);
              Out.begin(cfg_addrOut, cfg_freq);
              break;
            case 2:
              convert(buf, cfg_addrPodacha);
              Podacha.begin(cfg_addrPodacha, cfg_freq);
              break;
            case 3:
              convert(buf, cfg_addrObratka);
              Obratka.begin(cfg_addrObratka, cfg_freq);
              break;      
          }
        else warn += "<br />" + _addr + utf8rus(": дубль адреса");    
      }
      else warn += "<br />" + _addr + utf8rus(": Длина адреса не соответствует"); 
    }
  }
  String msg = "<body onload='setInterval(function(){ window.history.go(-2)}, 3000);'>" + utf8rus("Конфигурация успешно сохранена");
  if (warn.length() > 0)
    msg += "</ br>" + utf8rus("Но некоторые адреса не сохранены: ") + warn;
  msg += "</body>";
  if (saveConfig()) HTTP.send(200, "text/html", msg.c_str());
  else HTTP.send(200, "text/html", utf8rus("Конфиг не сохранен  <a href='javascript:history.go(-1)'>" + utf8rus("Назад") + "</a>").c_str());
}

void handle_Root() {
  String serverIndex;
  unsigned long timenagr;
  HTTP.sendHeader("Connection", "close");
  HTTP.sendHeader("Access-Control-Allow-Origin", "*");
  if (!morning) {
    if (tenLvl > 0) timenagr = time_nagrev_today + (millis() - time_on_nagr ) * tenLvl / 1000;
    else timenagr = time_nagrev_today;
  }
  else {
    if (tenLvl > 0) timenagr = time_nagrev_today + (millis() - time_on_nagr ) * tenLvl / 1000;
    else timenagr = time_nagrev_today;
  }
  serverIndex = "<h1 align=center>" + String(MODULE_NAME) + " " + String(MODULE_VERSION) + "  -  " + GetTime() + "</h1></br><h4>";
  serverIndex += utf8rus("Аптайм: ") + String(( millis() / 3600000 ) / 24) + utf8rus(" дней ") + Uptime() + "</br>";
  serverIndex += utf8rus("Последняя синхронизация NTP: ") + LastSync() + "</br></br>";
  serverIndex += utf8rus("Время работы котла сейчас " + String((morning)? "днем: ": "ночью: ")) + timeToStr((timenagr / 3600) % 24, (timenagr / 60) % 60, timenagr % 60) + "</br>";
  serverIndex += utf8rus("Сожрано электричества на сейчас " + String((morning)? "день: ": "ночь: ")) + String((float)round(timenagr / 36) * 3 / 100) + utf8rus(" кВт*ч") + "</br></br>";  
  serverIndex += utf8rus("Время работы котла прошлой ночью: ") + timeToStr((time_nagrev_yesterday / 3600) % 24, (time_nagrev_yesterday / 60) % 60, time_nagrev_yesterday % 60) + "</br>";
  serverIndex += utf8rus("Сожрано электричества прошлой ночью: ") + String((float)round(time_nagrev_yesterday / 36) * 3 / 100) + utf8rus(" кВт*ч") + "</br></br>";
  serverIndex += utf8rus("Время работы котла прошлым днем: ") + timeToStr((time_nagrev_morning / 3600) % 24, (time_nagrev_morning / 60) % 60, time_nagrev_morning % 60) + "</br>";
  serverIndex += utf8rus("Сожрано электричества прошлым днем: ") + String((float)round(time_nagrev_morning / 36) * 3 / 100) + utf8rus(" кВт*ч") + "</br>";
  serverIndex += "</h4><h2 align=center>" + utf8rus("Температура") + "</h2></br><table border='1' align=center><tr><td>" + utf8rus("Название") + "</td><td>" + utf8rus("Значение");
  serverIndex += "</td><td>" + utf8rus("Цель")+"</td></tr><tr><td>" + utf8rus("Котел") + "</td><td>" + String(Podacha.getTemp(), 1) + "</td><td>" + t_boiler + "</td></tr><tr><td>" + utf8rus("Обратка");
  serverIndex += "</td><td>" + String(Obratka.getTemp(), 1) + "</td><td></td></tr><tr><td>" + utf8rus("Дома") + "</td><td>" + String(In.getTemp(), 1) + "</td><td>" + String(temp_in_target) + "</td></tr><tr><td>";
  serverIndex += utf8rus("Улица") + "</td><td>" + String(Out.getTemp(), 1) + "</td><td></td></tr><tr><td>" + utf8rus("Скорость насоса") + "</td><td>" + String(speedNasos) + "</td><td></td></tr><tr><td>";
  serverIndex += utf8rus("Мощность котла") + "</td><td>" + String(tenLvl) + "</td><td></td></tr></table></br><a href='/config'>" + utf8rus("Настройки");
  serverIndex += "</a></br><form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  HTTP.send(200, "text/html", serverIndex);
}

void handle_Config() {
  String serverIndex2;
  HTTP.sendHeader("Connection", "close");
  HTTP.sendHeader("Access-Control-Allow-Origin", "*");
  serverIndex2 += "<head><style> h4 {margin-block-end: 0em; margin-block-start: 0.6em;}</style></head>";
  serverIndex2 += "<h1>" + utf8rus("Настройки ") + String(MODULE_NAME) + "</h1>" + "<form method='POST' action='/saveconf' enctype='multipart/form-data'>";
  serverIndex2 += "<br /><h4>" + utf8rus("Настройки WiFi") + "</h4>";
  serverIndex2 += "<label>SSID: <input type='text' name='ssid' value=" + String(cfg_ssid) + "></label> <br />";
  serverIndex2 += "<label>" + utf8rus("Пароль: ") + "<input type='password' name='pass' value=" + String(cfg_pass) + "></label><br />";
  serverIndex2 += "<h4>" + utf8rus("Настройки MQTT:") + "</h4>";
  serverIndex2 += "<label>" + utf8rus("Адрес MQTT сервера: ") + "<input type='text' name='mqtt_server' value=" + String(cfg_mqtt_server) + "></label><br />";  
  serverIndex2 += "<label>" + utf8rus("Порт MQTT: ") + "<input type='number' name='mqttport' value=" + String(cfg_mqttport) + "></label><br />";  
  serverIndex2 += "<label>" + utf8rus("Имя пользователя MQTT: ") + "<input type='text' name='userMqtt' value=" + String(cfg_userMqtt) + "></label><br />";  
  serverIndex2 += "<label>" + utf8rus("Пароль MQTT: ") + "<input type='password' name='passMqtt' value=" + String(cfg_passMqtt) + "></label><br />";  
  serverIndex2 += "<h4>" + utf8rus("Настройки времени:") + "</h4>";
  serverIndex2 += "<label>" + utf8rus("Тайм зона: ") + "<input type='text' name='timezone' value=" + String(cfg_timezone) + "></label><br />";  
  serverIndex2 += "<label>" + utf8rus("Адрес NTP сервера: ") + "<input type='text' name='ntp' value=" + String(cfg_ntpServerName) + "></label><br />";  
  serverIndex2 += "<label>" + utf8rus("Время ночи: ") + "<input type='number' name='night' value=" + String(cfg_time_night) + "></label> <br />";
  serverIndex2 += "<label>" + utf8rus("Время утра: ")+"<input type='number' name='morning' value=" + String(cfg_time_morning) + "></label> <br />";
  serverIndex2 += "<label>" + utf8rus("Конец отопительного сезона (месяц): ")+"<input type='number' name='endheatseason' value=" + String(cfg_eheats) + "></label> <br />";
  serverIndex2 += "<label>" + utf8rus("Начало отопительного сезона (месяц): ")+"<input type='number' name='begheatseason' value=" + String(cfg_bheats) + "></label> <br />";
  
// TODO - сделать чекбокс для подключения меркурия
  bool a = m_OpenCh();
  serverIndex2 += "<h4>" + String((a)? "<font color='#64FF00'>": "") + utf8rus("Подключение к счетчику Меркурий 230:") + String((a)? "</font>": "") + "</h4>";
  serverIndex2 += "<label>" + utf8rus("IP Адрес Меркурия: ") + "<input type='text' name='mhost' value=" + String(cfg_mhost) + "></label><br />";  
  serverIndex2 += "<label>" + utf8rus("Порт Меркурия: ") + "<input type='number' name='mport' value=" + String(cfg_mport) + "></label><br />";  
  String mpasss = "";
  for (int i = 0; i < 6; i++)
    mpasss += String(cfg_mpass[i], HEX);
  serverIndex2 += "<label>" + utf8rus("Пароль Меркурия: ") + "<input type='text' name='mpass' value=" + mpasss + "></label><br />";    
  serverIndex2 += "<h4>" + utf8rus("Настройки доступа к конфигурированию:") + "</h4>";
  serverIndex2 += "<label>" + utf8rus("Логин конфига: ")+"<input type='text' name='webuser' value='" + String(cfg_webuser) + "'></label><br />";
  serverIndex2 += "<label>" + utf8rus("Пароль конфига: ")+"<input type='password' name='webpass' value='" + String(cfg_webpass) + "'></label><br />";
  serverIndex2 += "<label>" + utf8rus("Время действия WEB сессии: ")+"<input type='number' name='websessionperiod' value='" + String(cfg_websessionperiod) + "'></label><br />";
  serverIndex2 += "<h4>" + utf8rus("Настройки температуры:") + "</h4>";
  serverIndex2 += "<label>" + utf8rus("Комфортная температура дома: ") + "<input type='number' name='t_compf' value=" + String(cfg_temp_comf) + "></label><br />";  
  serverIndex2 += "<label>" + utf8rus("Температура ночью дома: ") + "<input type='number' name='t_night' value=" + String(cfg_temp_night) + "></label><br />";
  serverIndex2 += "<label>" + utf8rus("Разница температур подачи и обратки: ") + "<input type='number' name='dt' value=" + String(cfg_dt) + "></label><br />";
  serverIndex2 += "<label>" + utf8rus("Гистерезис температуры (для центробежки): ") + "<input type='number' name='circ_hyst' value=" + String(cfg_circ_hyst) + "></label><br />";
  serverIndex2 += "<label>" + utf8rus("Гистерезис температуры (для ТЕНа): ") + "<input type='number' name='ten_hyst' value=" + String(cfg_ten_hyst) + "></label><br />";
  serverIndex2 += "<label>" + utf8rus("Минимальная температура в доме: ") + "<input type='number' name='temp_min' value=" + String(cfg_temp_min) + "></label><br />";
  serverIndex2 += "<label>" + utf8rus("Оптимальная температура подачи: ") + "<input type='number' name='t_opt_pod' value=" + String(cfg_t_opt_pod) + "></label><br />";  
  serverIndex2 += "<label>" + utf8rus("Минимальная температура подачи: ") + "<input type='number' name='t_min_pod' value=" + String(cfg_t_min_pod) + "></label><br />";
  serverIndex2 += "<label>" + utf8rus("Минимальная температура обратки: ") + "<input type='number' name='t_min_obr' value=" + String(cfg_t_min_obr) + "></label><br />";
  serverIndex2 += "<label>" + utf8rus("Порог температуры подачи для уведомления: ") + "<input type='number' name='t_hot' value=" + String(cfg_t_hot) + "></label> <br />";
  serverIndex2 += "<label>" + utf8rus("Адрес датчика внутри: ")+"<input type='text' name='inaddr' value='" + In.addrStr() + "'></label>  " + String(In.getTemp(), 1) + "<br />";
  serverIndex2 += "<label>" + utf8rus("Адрес датчика снаружи: ")+"<input type='text' name='outaddr' value='" + Out.addrStr() + "'></label> " + String(Out.getTemp(), 1) + "<br />";
  serverIndex2 += "<label>" + utf8rus("Адрес датчика подачи: ")+"<input type='text' name='podachaaddr' value='" + Podacha.addrStr() + "'></label> " + String(Podacha.getTemp(), 1) + " <br />";
  serverIndex2 += "<label>" + utf8rus("Адрес датчика обратки: ")+"<input type='text' name='obratkaaddr' value='" + Obratka.addrStr() + "'></label> " + String(Obratka.getTemp(), 1) + "<br />";
  serverIndex2 += "<input type='submit' value=" + utf8rus("Обновить") + "></form> <a href='javascript:history.go(-1)'>" + utf8rus("Назад") + "</a>";
  String tmp=getUnConfSensors();
  if (tmp.length()>1)
    serverIndex2 += "<h3>" + utf8rus("Несохраненные датчики: ") + "</h3>" + tmp;
  HTTP.send(200,"text/html", serverIndex2);
}
