#include ".\src\EncButton.h"

// 0    -  LCD
#define TEN1        2          //1 ТЭН
#define TEN2        1          //2 ТЭНа

#define ENC_A       5          //A - контакт энкодера
#define ENC_B       4          //B - контакт энкодера

#define NASOS1      5          //Контакт 1 скорости насоса
#define NASOS2      4          //Контакт 2 скорости насоса
#define MOTOR       3          //Контакт включения насоса

#define YELLOW_LED  7          //Контакт желтого светодиода 74HC
#define RED_LED     6          //Контакт красного светодиода 74HC




//----------------------------Строковые функции--------------------------------------
String humanTimeMillis(unsigned long milli) {
  String s;
  //unsigned long milli;
  //milli = millis();
  unsigned long secs = milli / 1000, mins = secs / 60;
  unsigned int hours = mins / 60, days = hours / 24;
  milli -= secs * 1000;
  secs -= mins * 60;
  mins -= hours * 60;
  hours -= days * 24;
  s += days != 0 ? (String)days : "";
  s += days != 0 ? "d " : "";
  s += hours != 0 ? (String)hours : "";
  s += hours != 0 ? ":" : "";
  s += mins > 9 ? "" : "0";
  s += mins;
  s += ":";
  s += secs > 9 ?  "" : "0";
  s += secs;
  return s;
}
//-----------------------------------------------------------------------------------
String utf8rus(String source) {
  int i, k;
  String target;
  unsigned char n;
  char m[2] = { '0', '\0' };

  k = source.length(); 
  i = 0;

  while (i < k) {
    n = source[i]; 
    i++;
    if (n >= 0xC0) {
      switch (n) {
        case 0xD0: {
          n = source[i]; i++;
          if (n == 0x81) { n = 0xA8; break; }
          if (n >= 0x90 && n <= 0xBF) n = n + 0x30;
          break;
        }
        case 0xD1: {
          n = source[i]; i++;
          if (n == 0x91) { n = 0xB8; break; }
          if (n >= 0x80 && n <= 0x8F) n = n + 0x70;
          break;
        }
      }
    }
    m[0] = n; 
    target = target + String(m);
  }
return target;
}
//----------------------------Конвертация char to byte-------------------------------
// str   - указатель на массив символов
// bytes - выходной буфер
// функция возвращает колл-во байт
//
int convert( char* str, unsigned char* bytes )
{
   unsigned char Hi, Lo;
   int i = 0;
   while( ( Hi = *str++ ) && ( Lo = *str++ ) )
   {
      bytes[i++] = ( toHex( Hi ) << 4 ) | toHex( Lo );
   }
   return i;
}
//----------------------------Установка скорости насоса------------------------------
void setNasosSpeed (byte speedN) {  
  if (speedNasos != speedN) {
    if (speedN == 0) {bitClear(hc74tmp,MOTOR); bitClear(hc74tmp,YELLOW_LED);}
    else {
      bitSet(hc74tmp, MOTOR);
      bitSet(hc74tmp, YELLOW_LED);
      if (speedN == 1) { bitSet(hc74tmp, NASOS1); bitClear(hc74tmp, NASOS2);}
      else {bitClear(hc74tmp, NASOS1); bitSet(hc74tmp, NASOS2);}
    }
    speedNasos = speedN;  
    HACirc.setValue(String(speedN).c_str());
  }  
}
//----------------------------Установка мощности ТЭНа--------------------------------
void setTenStep (byte Step) {      
  if (tenLvl != Step) {
    //------------------- Кусок отвечающий за подсчет трат энергии
    if (tenLvl != 0) {
      time_nagrev_today += (millis() - time_on_nagr ) * tenLvl / 1000;
    }
    time_on_nagr = millis();
    //------------------ окончание куска считающего траты энергии
    tenLvl = Step;
    HATen.setValue(String(tenLvl).c_str());
    if (tenLvl > 0) bitSet(hc74tmp, RED_LED);
    else bitClear(hc74tmp, RED_LED);
    hc74tmp &= 0b11111001;
    hc74tmp |= (Step << 1);
  }
}

//------ выбор ступени тена в зависимости от целевой температуры
void swTenLvl(int target) {
  switch (tenLvl) {
    case 3:
      if (int(Podacha.getTemp() / target * 100) >= 75 + cfg_ten_hyst)
        setTenStep(2);
      break;
    case 2:
      if (int(Podacha.getTemp() / target * 100) < 75 - cfg_ten_hyst)
        setTenStep(3);
      if (int(Podacha.getTemp() / target * 100) >= 90 + cfg_ten_hyst)
        setTenStep(1);
      break;
    case 1:
      if (int(Podacha.getTemp() / target * 100) < 90 - cfg_ten_hyst)
        setTenStep(2);                
      if (int(Podacha.getTemp() / target * 100) >= 100 + cfg_ten_hyst) {                
        setTenStep(0);
      }  
      break;
    case 0:
      if (int(Podacha.getTemp() / target * 100) < 100 - cfg_ten_hyst) {
        setTenStep(1);
      }  
      break;
  }
}


//------------------------------Управление центробежкой------------------------------
void CircLoop() {
    //Если электрическое отопление
      switch (speedNasos) {
        case 2:
          if ((!morning and ((Podacha.getTemp() < cfg_t_hot - cfg_circ_hyst) or (Podacha.getTemp() - Obratka.getTemp() < cfg_dt * 2 - cfg_circ_hyst))) or (morning and (Podacha.getTemp() < cfg_t_hot - cfg_dt - cfg_circ_hyst)))
            setNasosSpeed(1);
          break;
        case 1:
          if ((!morning and ((Podacha.getTemp() >= cfg_t_hot) or (Podacha.getTemp() - Obratka.getTemp() >= cfg_dt * 2 + cfg_circ_hyst))) or (morning and (Podacha.getTemp() >= cfg_t_hot - cfg_dt + cfg_circ_hyst)))
            setNasosSpeed(2);
          if ((!morning and ((Podacha.getTemp() - Obratka.getTemp() < cfg_dt - cfg_circ_hyst))) or (morning and (Podacha.getTemp() < cfg_t_opt_pod - cfg_circ_hyst)))
            setNasosSpeed(0);
          break;
        case 0:
          if ((!morning and ((Podacha.getTemp() - Obratka.getTemp() >= cfg_dt + cfg_circ_hyst))) or (morning and (Podacha.getTemp() >= cfg_t_opt_pod + cfg_circ_hyst)))
            setNasosSpeed(1);
          break;
      }
}    
//----------------------------Управление ТЕНом-----------------------------------------
void TenLoop() {
  if(ntpSync){ //если получили время из интернета
    byte h = GetPartOfTime(4);
    byte month = GetPartOfTime(1);
//------------------Расчет температур---------------    
// Дисплейная функция      UpdateTarget();
//--------------Электрический нагрев----------------
    if ((month <= cfg_bheats or month >= cfg_eheats)) {
      if (!(h >= cfg_time_morning && h < cfg_time_night)) {   //Если ночь, включаем электричество!
        if (morning) {
          morning = false;
          time_nagrev_morning = time_nagrev_today + ( (tenLvl > 0)? (millis() - time_on_nagr) * tenLvl / 1000: 0);
          HATimeM.setValue(String(time_nagrev_morning).c_str());
          HAEnergyMorning.setValue(String((float) round(time_nagrev_morning / 36) * 3 / 100).c_str());
          time_nagrev_today = 0;   
        }
        if (h > 1 && h < 5) (temp_in_target = cfg_temp_night);
        else (temp_in_target = cfg_temp_comf);
        // TO DO Надо поискать как рассчитать температуру теплоносителя
        t_boiler = int(20 + (temp_in_target - Out.getTemp()) + (temp_in_target - In.getTemp()) * 30);
        if (t_boiler >= cfg_t_hot) t_boiler = cfg_t_hot;
        if (t_boiler <= cfg_t_min_pod) t_boiler = cfg_t_min_pod;
        swTenLvl(t_boiler);  
      }     
      else {//Если день, то надо думать...
        if (!morning) { 
          morning = true;
          time_nagrev_yesterday = time_nagrev_today + ( (tenLvl > 0)? (millis() - time_on_nagr) * tenLvl / 1000: 0);
          HATime.setValue(String(time_nagrev_yesterday).c_str());
          HAEnergyNight.setValue(String((float) round(time_nagrev_yesterday / 36) * 3 / 100).c_str());
          time_nagrev_today = 0;
        } 
        if (t_boiler != cfg_t_min_pod) t_boiler = cfg_t_min_pod;
        swTenLvl(t_boiler);          
      }
    }
  }  
  else { //Если вермя не получили
    setTenStep(0);
// TO DO -------------НАДО получить время из меркурия!!!!!, но в настройках должен быть чекбокс о том что его нужно использовать
  }
}  


// TODO  -  пересмотреть механизм!!! датчики троят, когда сильно часто спрашиваешь их, плюс нужно пересмотреть перезапуск при отвале датчика!
void GetTemp() {
  float temp;
  
  if (!Podacha.conn) Podacha.searchDS(); // если датчик отвалился заново бегин
  else {
    temp = Podacha.getTemp();
    if ((temp != Podacha.prevTemp()) and (temp > - 60)) HApodacha.setValue(String(temp).c_str());
  }
  if (!Obratka.conn) Obratka.searchDS();
  else {
    temp = Obratka.getTemp();
    if ((temp != Obratka.prevTemp()) and (temp > - 60)) HAobratka.setValue(String(temp).c_str());
  }
  if (!In.conn) In.searchDS();
  else {
    temp = In.getTemp();
    if ((temp != In.prevTemp()) and (temp > - 60)) HAinside.setValue(String(temp).c_str());
  }
  if (!Out.conn) Out.searchDS();
  else {
    temp = Out.getTemp();
    if ((temp != Out.prevTemp()) and (temp > - 60)) HAoutside.setValue(String(temp).c_str());
  }
}

void KotelLoop() {
  if ((cur_ms - mstemper) >= temperPeriod) {
    mstemper = cur_ms;
    GetTemp();
    if (Podacha.conn and Obratka.conn and In.conn and Out.conn ) {  
      if (AlarmEn) AlarmEn = false;
      hc74tmp = hc74;
      CircLoop();
      TenLoop();
      if (hc74 != hc74tmp) {
        hc74 = hc74tmp;
        writeHC74(hc74);
      }
    }
    else AlarmEn;
  }
}

//----------------------------Управление синхронизацией времени------------------------

void Sens_init() {
  if (cfg_addrIn[0] == 0x28)
    In.begin(cfg_addrIn, cfg_freq);
  if (cfg_addrOut[0] == 0x28)
    Out.begin(cfg_addrOut, cfg_freq);
  if (cfg_addrPodacha[0] == 0x28)
    Podacha.begin(cfg_addrPodacha, cfg_freq);
  if (cfg_addrObratka[0] == 0x28)
    Obratka.begin(cfg_addrObratka, cfg_freq);
  dspLogOut(" Датчик дома: " + String((!In.conn)? " откл.": In.addrStr()));
  dspLogOut(" Датчик улица: " + String((!Out.conn)? " откл.": Out.addrStr()));
  dspLogOut(" Датчик подача: " + String((!Podacha.conn)? " откл.": Podacha.addrStr()));
  dspLogOut(" Датчик обратка: " + String((!Obratka.conn)? " откл.": Obratka.addrStr()));
}

unsigned long LastAlarmTick = 0;
bool bz = 0;
void AlarmTick() {
  if (AlarmEn) {
    if (millis() - LastAlarmTick >= 1000) {
      bz = !bz;
      LastAlarmTick = millis();
      if (bz)
        tone(BUZZ, 500);
      else noTone(BUZZ);
    }
  }
  else noTone(BUZZ);
}

//----------------------------ENCODER---------------------------
EncButton<EB_CALLBACK, ENC_A, ENC_B, ENC_C> enc;
void encInit() {
  enc.attach(RIGHT_HANDLER, encRight);
  enc.attach(LEFT_HANDLER, encLeft);  
  enc.attach(RIGHT_H_HANDLER, encRightH);
  enc.attach(LEFT_H_HANDLER, encLeftH);
  enc.attach(CLICKS_HANDLER, encClicks);
}  

// TODO manual powerOn on not night time
void onPowerCommand(bool state, HAHVAC* sender) {
  if (state) {
    int a = 0;
  }
}

void onModeCommand(HAHVAC::Mode mode, HAHVAC* sender) {
    Serial.print("Mode: ");
    if (mode == HAHVAC::OffMode) {
        Serial.println("off");
    } else if (mode == HAHVAC::AutoMode) {
        Serial.println("auto");
    } else if (mode == HAHVAC::CoolMode) {
        Serial.println("cool");
    } else if (mode == HAHVAC::HeatMode) {
        Serial.println("heat");
    } else if (mode == HAHVAC::DryMode) {
        Serial.println("dry");
    } else if (mode == HAHVAC::FanOnlyMode) {
        Serial.println("fan only");
    }

    sender->setMode(mode); // report mode back to the HA panel
}

void encRightH() {
  dspLogOut("RIGHT_H_HANDLER");
}
void encLeftH() {
  dspLogOut(String(GetPartOfTime(2)));
}
void encRight() {
  dspLogOut(String(GetPartOfTime(1)));
}
void encLeft() {
 
}
void encClicks() {
  
  if (enc.clicks == 8) {
    resetConfig();
    delay(1000);
    ESP.restart();
  }
  if (enc.clicks == 5) {
    ESP.restart();  
  }
  if (enc.clicks == 1) {
    lcdWake();
  }
}

void EncTick() {
  enc.tick();
}
