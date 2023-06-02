//---------------commands--------------------------------
byte cmd_conn[] = {0x46, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte cmd_time[] = {0x46, 0x04, 0x00, 0x92, 0xd5};
// 46 49 00 12 02 02 11 21 01 26 56

//------------------------Client init---------------------------------------
WiFiClient mercury;

bool m_OpenCh() {
  if (!mercury.connected()) {
    if (!mercury.connect(cfg_mhost, cfg_mport)) 
    {
      DBG_PRN("Connection to mercury failed");
      return false;
    }
    else {
      if (mercury.connected()) {
        DBG_PRN("Connection to Mercury ok"); 
        for (byte i = 0; i < 6; i++)
          cmd_conn[i + 3] = cfg_mpass[i];
        CRC16(cmd_conn, 9);
        mercury.write(cmd_conn, sizeof(cmd_conn));    
        unsigned long timeOut = millis();
        while (!mercury.available()) {
          delay(100);
          if (millis() - timeOut > 2000) return false;
        }
        int i = 0;  
        byte reply[4];
        while (mercury.available()) {
          reply [i++] = mercury.read();
        }
        if (reply[1] == 0x00) return true;
        else return false;
      }  
      else return false;
    }
  }
}

//void testbrute() {
//  if (!mercury.connected()) {
//    if (!mercury.connect(hosttcp, porttcp)) 
//    {
//      DBG_PRN("Connection to mercury failed");
//      Serial.println("Not connected");
//    }
//    else {
//      if (mercury.connected()) {
//        mercury.write(cmd_conn, sizeof(cmd_conn));    
//        unsigned long timeOut = millis();
//        while (!mercury.available()) {
//          delay(1);
//        }        
//        Serial.println(String((millis() - timeOut)/1000));
//      }
//    } 
//  }   
//}

//void brute() {
//  byte pass[6]={0xaa,0xaa,0xaa,0xaa,0xaa,0xaa};
//  if (!mercury.connected()) {
//    if (!mercury.connect(hosttcp, porttcp)) 
//    {
//      DBG_PRN("Connection to mercury failed");
//      Serial.println("Not connected");
//    }
//    else {
//      DBG_PRN("Connection to Mercury ok"); 
//      if (mercury.connected()) {
//        byte brut[] = {0x46, 0x01, 0x02, 1, 1, 1, 1, 1, 1, 0, 0};
//        byte a = 0;
//          for (byte b = 0; b < 10; b++)
//            for (byte c = 0; c < 10; c++) {
//              Serial.println("------ " + String(a) + "  " + String(b) + "  " + String(c));
//              for (byte d = 0; d < 10; d++) {
//                for (byte e = 0; e < 10; e++)
//                  for (byte f = 0; f < 10; f++) {
//                    bool rep = true;
//                    brut[3]=a;
//                    brut[4]=b;
//                    brut[5]=c;
//                    brut[6]=d;
//                    brut[7]=e;
//                    brut[8]=f;
//                    CRC16(brut, 9);
//                    mercury.write(brut, sizeof(brut));    
//                    unsigned long timeOut = millis();
//                    while (!mercury.available()) {
//                      delay(60);
//                      if (millis() - timeOut > 240) { rep = false; break; }
//                    }
//                    if (rep) {
//                      int i = 0;  
//                      byte reply[] = {1,1,1,1};
//                      while (mercury.available()) {
//                        reply [i++] = mercury.read();
//                      }
//                      if (reply[1] == 0x00) {
//                        Serial.println("======================================");
//                        for (byte j = 3; j < 9; j++) {
//                          Serial.println(brut[j], HEX);
//                          pass[j-3] = brut[j];
//                        }  
//                        Serial.println("======================================");
//                        break;    
//                      }
//                    }
//                  }
//              }    
//            }
//      }  
//      else DBG_PRN("Something wrong");
//    }
//  }
//                      Serial.println("======================================");
//                      for (byte j = 0; j < 6; j++)
//                        Serial.println(pass[j], HEX);
//                      Serial.println("======================================");
//}

void m_TimeRead() {
  if (m_OpenCh()) {
    mercury.write(cmd_time, sizeof(cmd_time));  
    byte reply[10];
    unsigned long timeOut = millis();
    while (!mercury.available()) {
      delay(100);
      if (millis() - timeOut > 2000) return;
    }
    byte i = 0;  
    while (mercury.available()) {
      reply[i++] = bcdToDec(mercury.read());
    }

    // 3 - Часы
    // 2 - Минуты
    // 1 - Секунды
    // 5 - День
    // 6 - Месяц
    // 7 - Год
    
    DBG_PRN("HOUR: " + String(reply[3]) + " Minutes: " + String(reply[2]) + " Seconds: " + String(reply[1]));
  }
}


// TO DO - создать функцию коррекции времени на счетчике

void m_TimeCorrect () {
  
}

void CRC16(uint8_t  frame[], uint8_t len) {
  uint16_t crcReg = 0xFFFF; // создаем и заполняем регистр единицами 
  for (uint8_t i = 0; i < len; i++){ // для количества байт нашего запроса
    crcReg ^= frame[i]; // исключающее ИЛИ (сумма по модулю 2) с содержимым регистра с присвоением 
    for (uint8_t j=0; j<8; j++){ // для каждого бита байта
      if (crcReg & 0x01) { // если младший бит равен единице 
        crcReg = (crcReg >>=1) ^ 0xA001; // сдвигаем вправо на 1 бит и исключающее ИЛИ (сумма по модулю 2) с полиномиальным числом 0xA001    
        }
      else crcReg >>= 1; // если младший бит равен нулю - просто сдвиг
   }
}
// после окончания циклов в переменной crcReg лежит наша контрольная сумма
// поменяем байты местами и поместим ее в массив crc16stor
//  *crc16stor = crcReg & 0x00FF; // сначала выполним логическое умножение (И) содержимого регистра на число 0x00FF, тем самым получим младший байт и разместим его в нулевом члене массива 
//  crc16stor ++; // переместим указатель на 1 первый член массива
//  *crc16stor = crcReg >> 8; // сдвинем содержимое регистра вправо на 8 бит (1 байт) и получим старший байт. Разместим его в первом члене массива. 
  frame[9] = crcReg & 0x00FF; // сначала выполним логическое умножение (И) содержимого регистра на число 0x00FF, тем самым получим младший байт и разместим его в нулевом члене массива 
  frame[10] = crcReg >> 8; // сдвинем содержимое регистра вправо на 8 бит (1 байт) и получим старший байт. Разместим его в первом члене массива. 

}

byte bcdToDec(byte val)
{
  return( (val/16*10) + (val%16) );
}

inline unsigned char toHex( char ch )   //Для меркурия
{
   return ( ( ch >= 'A' ) ? ( ch - 'A' + 0xA ) : ( ch - '0' ) ) & 0x0F;
}
