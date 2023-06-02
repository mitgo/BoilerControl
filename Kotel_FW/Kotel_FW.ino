//FS: 2MB, OTA: 1024 kb
#include <ESP8266WiFi.h>        //Содержится в пакете. Видео с уроком http://esp8266-arduinoide.ru/step1-wifi
#include <ArduinoHA.h>
#define BUZZ        1
#define ONWORK 0
#if (ONWORK == 9)
#define DBG 1
#endif

#ifdef DBG
#define DBG_PRN(x) Serial.println(x)
#else
#define DBG_PRN(x)
#endif

#define ENC_C       15         //кнопка энкодера

#define MODULE_NAME "BoilerControl"
#define MODULE_VERSION "0.72"


//--------------------DS18B20----------------------
class tempSensor {
  public:
    //tempSensor (int id, int freeq) { begin(id, freeq); }
    void begin(byte *addr, int freeq);
    void searchDS();
    void prepare();
    float getTemp();
    float prevTemp(); 
    String addrStr();
    unsigned long lastSucGet();
    bool conn;   // подключен?
  
  private:
    byte _addr[8];
    byte _curr;    //индекс текущей температуры в массиве температур
    int _temparr[5];  //массив температур (храним целые, в 10 раз больше)
    byte _freeqScan;   //частота сканирования
    unsigned long _lastScan;     // Время последнего сканирования
    unsigned long _lastSuccessGet;
    byte _readReq;
    
};

byte            cfg_freq=5;
                tempSensor In;
                tempSensor Out;       
                tempSensor Podacha;
                tempSensor Obratka;       

//------------------Конфигурационные переменные-----------------------------------------------------
byte cfg_temp_comf = 23;
byte cfg_temp_night = 20;
byte cfg_temp_min = 19;
byte cfg_dt = 10;
byte cfg_circ_hyst = 3;
byte cfg_ten_hyst = 2;
byte cfg_t_min_pod = 15;
byte cfg_t_opt_pod = 60;
byte cfg_t_min_obr = 15;
byte cfg_t_hot = 88;
byte cfg_time_morning = 7;
byte cfg_time_night = 23;
byte cfg_bheats = 9;
byte cfg_eheats = 5;
bool cfg_STAen=true;
char cfg_ssid[25];         //SSID роутера
char cfg_pass[15];       //Пароль роутера
char cfg_lastGetIp[15] = "";
char cfg_lastGetMask[15] = "";
char cfg_lastGetGW[15] = "";
char cfg_webuser[10];
char cfg_webpass[10];

byte cfg_tries;
int cfg_websessionperiod;

byte hc74tmp = 0;


char            cfg_ntpServerName[30] = "time.nist.gov";                
unsigned int    cfg_ntpPeriod = 3600 * 1;
byte            cfg_timezone = 9; 

byte cfg_addrIn[] = {0, 0, 0, 0, 0, 0, 0, 0};
byte cfg_addrOut[] = {0, 0, 0, 0, 0, 0, 0, 0};
byte cfg_addrPodacha[] = {0, 0, 0, 0, 0, 0, 0, 0};
byte cfg_addrObratka[] = {0, 0, 0, 0, 0, 0, 0, 0};

byte            hc74 = 0;
byte            speedNasos = 0;
byte tenLvl;

byte temp_in_target;
int t_boiler; // ИНТ - потому что может быть отрицательным!!!
bool lcdOn;

bool ntpSync=false;

//---------------------------------MQTT--------------------------
#ifndef MQTT_USER
#define MQTT_USER "YOUR_MQTT_USER"
#endif
#ifndef MQTT_PASS
#define MQTT_PASS "YOUR_MQTT_PASS"
#endif
char cfg_userMqtt[15] = MQTT_USER; // replace with your credentials
char cfg_passMqtt[15] = MQTT_PASS;
char cfg_mqtt_server[16] = "192.168.1.2"; // Имя сервера MQTT
int cfg_mqttport = 1883;
int cfg_mqttperiod;

//------------------------------Mercury 230-------------------
#ifndef MERCURY_PORT
#define MERCURY_PORT 8826
#endif
#ifndef MERCURY_HOST
#define MERCURY_HOST "85.88.163.30"
#endif
int cfg_mport = MERCURY_PORT;
char cfg_mhost[16] = MERCURY_HOST;
byte cfg_mpass[6] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

//------------------TIMERS--------------------------
unsigned long time_on_nagr, time_nagrev_today;
unsigned int time_nagrev_yesterday, time_nagrev_morning;
unsigned long cur_ms   = 0;
unsigned long msdisp      = 10000000UL;          // время с последнего отображения времени на дисплее
unsigned long mssync      = 10000000UL; // время с последней синхронизации
unsigned long msconn      = 0;          // время с последней попытки подключения
unsigned long msshut      = 0;
unsigned long msmqtt      = 0;
unsigned long t_cur    = 0;
unsigned long temperPeriod=10000, mstemper = 0;
//bool          points   = true;
unsigned int err_count = 0;

bool morning = true;
bool AlarmEn = false;
byte logLine=0;

//-------------------===MQTT===----------------------
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
HAHVAC hvac("TargetTemp", HAHVAC::TargetTemperatureFeature | HAHVAC::PowerFeature | HAHVAC::ModesFeature);
HASensor HApodacha("BoilerPodacha");
HASensor HAobratka("BoilerObratka");
HASensor HAinside("HomeInside");
HASensor HAoutside("HomeOutside");
HASensor HACirc("BoilerCirc");
HASensor HATen("BoilerTEN");
HASensor HATime("TimeHeat");
HASensor HATimeM("TimeHeatMorning");
HASensor HAEnergyNight("EnergyNight");
HASensor HAEnergyMorning("EnergyMorning");
//**********************************************************************
// SETUP
//********************************************************************** 
void setup() {
  delay(300);
  spiInit();
  #ifdef DBG
    Serial.begin(115200);
    DBG_PRN("");
  #else
    pinMode(BUZZ, OUTPUT);
  #endif
  encInit();
  dispInit();
  dspLogOut("Загружаю конфиг");
  eepromInit();
  dspLogOut("Запускаю WI-FI");
  WIFIinit();
  dspLogOut("Запускаю HTTP");
  HTTP_init();   
  dspLogOut("Запускаю NTP");
  NTPInit();
  dspLogOut("Инициализирую сенсоры");
  Sens_init();
  dspLogOut("Запускаю MQTT");
  MqttInit();  
  ClrScr();
  DispTempLabel();
  DispTemp(true);
  logLine=0;
}

void loop() {
  cur_ms = millis();
  KotelLoop();        //Управление котлом
  AlarmTick();
  processAPcheck();   //Проверяем - подключены или нет к сети интернет
  HTTP_loop();        //Функции http
  NTP_loop();         //Функции времени
  mqtt.loop();
  ScreenLoop();       // Управление обновлением экрана
  EncTick();
}
