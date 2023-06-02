#ifndef defSsid
#define defSsid         "YOUR_SID"
#define defPasswd       "YoUr_PaSs"
#endif

#define defPasswdAP     "12345678"
#define apCheckTickTime 60000

IPAddress apIP(192, 168, 1, 1);
IPAddress ipSTA;
//unsigned long nextapCheckTick;
unsigned long apCheckLastTick;

//----------------------WIFIinit--------------------------------------------
void WIFIinit() {
  WiFi.disconnect(true);
  WiFi.persistent(false);
  //WiFi.setPhyMode(WIFI_PHY_MODE_11G);
  delay(500);
  WiFi.onEvent(WiFiEvent);
  if (!cfg_STAen or !StartSTAMode()) {
    dspLogOut("Запускаю точку доступа"); 
    DBG_PRN("Starting AP mode"); 
    StartAPMode();
  }
}
//----------------------StartSTAMode--------------------------------------------
bool StartSTAMode()
{
    byte triess=cfg_tries;
    apCheckLastTick = millis();
    dspLogOut("Подключаюсь к " + String(cfg_ssid)); 
    DBG_PRN("Try connect to "+String(cfg_ssid)); 
    //Проверяем - есть ли точка с таким ССайДИ
    if (checkAPinair(cfg_ssid))
    {
      // Если есть такая точка - пытаемся подключиться
      WiFi.mode(WIFI_STA);
      WiFi.hostname(MODULE_NAME);
      WiFi.begin(cfg_ssid, cfg_pass);
      //Делаем проверку подключения до тех пор пока счетчик tries
      // не станет равен нулю или не получим подключение
      // НУЖНО ТОЛЬКО ДЛЯ ОТЛАДКИ
      while (--triess && WiFi.status() != WL_CONNECTED)
      {
          delay(1000);
          DBG_PRN(".");
          dspLogOut(".");
//------------------ДОБАВИТЬ ВЫВОД НА ДИСПЛЕЙ ПРОЦЕССА ПОДКЛЮЧЕНИЯ---------------          
      }
      if (WiFi.status() != WL_CONNECTED)
      {
        dspLogOut("Не могу подключитсья к " + String(cfg_ssid));
        DBG_PRN("Can't connect to SSID: " + String(cfg_ssid));
        return false;
      }
      else 
      {
        // Иначе удалось подключиться отправляем сообщение
        // о подключении и выводим адрес IP
     /*   if (WiFi.getAutoConnect() != true)    //configuration will be saved into SDK flash area
        {
          WiFi.setAutoConnect(true);   //on power-on automatically connects to last used hwAP
          WiFi.setAutoReconnect(true);    //automatically reconnects to hwAP in case it's disconnected
        }       
     */
        ipSTA = WiFi.localIP();
        dspLogOut("Подключен к " + String(cfg_ssid) + " " + ipSTA.toString()); 
        drawWifi(WiFi.RSSI());
        DBG_PRN("Connected to " + String(cfg_ssid) + " " + ipSTA.toString()); 
        return true;        
      }
    }
    else {
     dspLogOut("Нет точки " + String(cfg_ssid)); 
     DBG_PRN("No " + String(cfg_ssid) + " Net."); 
     return false;
    } 
}

//--------------------------------------StartAPMode------------------------------------
bool StartAPMode()
{ // Отключаем WIFI
  WiFi.disconnect();
  WiFi.mode(WIFI_AP);
  // Задаем настройки сети
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  // Включаем WIFI в режиме точки доступа с именем и паролем
  // хранящихся в переменных _ssidAP _passwordAP
  if (WiFi.softAP(MODULE_NAME, defPasswdAP)) {
    dspLogOut("Запущена точка " + String(MODULE_NAME)); 
    DBG_PRN("AP " + String(MODULE_NAME) + " started"); 
    return true;
  }  
  else 
    return false;
}

//-------------------------------------processAPcheck----------------------------------
void processAPcheck() {
  if (cfg_STAen && WiFi.status() != WL_CONNECTED)
  {
    if (millis() - apCheckLastTick >= apCheckTickTime * 1000) // nextapCheckTick
    {
      if (!StartSTAMode())
        StartAPMode();
      apCheckLastTick = millis();
    }
  }
}

// ----------------------------------- checkAPinair -----------------------------------
boolean checkAPinair(String name) {
  name.toUpperCase();
  int n = WiFi.scanNetworks();
  if (n == 0)
    return false;
  else
  {
    String nnn;
    for (int i = 0; i < n; ++i)
    {
      nnn = WiFi.SSID(i);
      DBG_PRN(nnn + ", RSSI: " + WiFi.RSSI(i)); 
      nnn.toUpperCase();
      if (nnn == name)
      {
        dspLogOut("Сеть: " + nnn + " найдена, RSSI: " + WiFi.RSSI(i)); 
        DBG_PRN(nnn + " network found, RSSI: " + WiFi.RSSI(i)); 
        return true;
      }
    }
  }
  return false;
}

// ----------------------------------- wifiEvent -----------------------------------
void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case WIFI_EVENT_STAMODE_GOT_IP:
        ipSTA = WiFi.localIP();

        if (String(cfg_lastGetIp) != ipSTA.toString()) {
          strncpy(cfg_lastGetIp, ipSTA.toString().c_str(), sizeof(cfg_lastGetIp));
          strncpy(cfg_lastGetGW, WiFi.gatewayIP().toString().c_str(), sizeof(cfg_lastGetGW));
          strncpy(cfg_lastGetMask, WiFi.subnetMask().toString().c_str(), sizeof(cfg_lastGetMask));
          saveIpSettings();
        }
        
      break;
    case WIFI_EVENT_STAMODE_DISCONNECTED:

      break;
    case WIFI_EVENT_STAMODE_DHCP_TIMEOUT:
    //TO DO - сделать стат настройки и неконнектед, и если со стат настройками борода - ап моде
      IPAddress ip;
      ip.fromString(String(cfg_lastGetIp));
      WiFi.config(ip.fromString(String(cfg_lastGetIp)), ip.fromString(String(cfg_lastGetGW)), ip.fromString(String(cfg_lastGetGW)), ip.fromString(String(cfg_lastGetMask)));
      break;      
      /*WIFI_EVENT_STAMODE_CONNECTED = 0,
        WIFI_EVENT_STAMODE_DISCONNECTED,1
        WIFI_EVENT_STAMODE_AUTHMODE_CHANGE,2
        WIFI_EVENT_STAMODE_GOT_IP,3
        WIFI_EVENT_STAMODE_DHCP_TIMEOUT,4
        WIFI_EVENT_SOFTAPMODE_STACONNECTED,5
        WIFI_EVENT_SOFTAPMODE_STADISCONNECTED,6
        WIFI_EVENT_SOFTAPMODE_PROBEREQRECVED,7
        WIFI_EVENT_MAX*/
  }
}
