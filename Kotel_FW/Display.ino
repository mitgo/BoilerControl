#include ".\src\Adafruit_GFX.h"           // Графическая библа
#include ".\src\Adafruit_ILI9341.h"       // LCD
#include ".\src\Fonts\FreeSerif9pt7b.h"   //Кастомный шрифт
#include ".\src\Fonts\FreeSans8.h"   //Кастомный шрифт
#include ".\src\Fonts\font.h"

#define TFT_CS      0               //чипселект дисплея
#define TFT_DC      2               //данные дисплея
#define WIDTH       320             //Ширина Экрана
#define HEIGH       240             //Высота экрана
#define FONT_SIZE   2
#define TEXT_W      6 * FONT_SIZE   //Базовая ширина символа
#define TEXT_H      8 * FONT_SIZE   //Базовая высота символа
#define CUSTOM_TEXT_H 23
#define CUSTOM_TEXT_W 16
#define STATUS_H TEXT_H + 1
#define LCD_LED         0           //Пин 74HC595 к которому подключен LCD

/*
 * ESP8266-12        HY-1.8 SPI
 * RESET to VCC
 * GPIO2             Pin 07 (A0)
 * GPIO13 (HSPID)    Pin 08 (SDA)   MOSI, DS
 * GPIO14 (HSPICLK)  Pin 09 (SCK)   SH, CLK
 * GPIO0  (HSPICS)   Pin 10 (CS)    TFT_CS
  */

//--------------------COLOURS------------------------
//#define ILI9341_ORANGE  0x1fF 
uint16_t status_bg =       ILI9341_WHITE;
uint16_t status_txt =      ILI9341_BLACK;
uint16_t status_warn =     ILI9341_RED;
uint16_t txtс =            ILI9341_WHITE;
uint16_t bg =              ILI9341_BLACK;

Adafruit_ILI9341 display = Adafruit_ILI9341(TFT_CS, TFT_DC);

void dispInit() {
  display.begin(40000000);
  display.cp437(true);
  display.fillScreen(ILI9341_BLACK);
  display.setRotation(0);
  display.setTextWrap(true);
  display.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  ClrStatus(0,0);
  dspTime();
  drawNTP(ntpSync);
  bitSet(hc74, LCD_LED);
  writeHC74(hc74);
  lcdOn=true;
}

void DispText(String text, uint16_t color, uint16_t bgcolor, int pt, int xpos, int ypos) {
  display.setTextWrap(false);
  display.setCursor(xpos, ypos);
  display.setTextColor(color, bgcolor);
  display.setTextSize(pt);
  display.println(utf8rus(text));
}

void DispMsg(String text, uint16_t color, int pt, int xpos, int ypos) {
  DispText(utf8rus(text),color,bg, pt, xpos, ypos);
}
void ClrStatus(int x1, int x2) {
  if (x2 == 0) x2 = WIDTH;
  display.fillRect(x1, 0, x2, STATUS_H, status_bg); 
}

void ClrScr() {
  display.fillRect(0, STATUS_H, WIDTH, HEIGH, bg); 
}

void DispTempLabel() {
  display.setFont(&FreeSans8pt8b);
  DispMsg("Котел", ILI9341_GREEN, FONT_SIZE + 1, 0, STATUS_H + 1 + 8);
  DispMsg("Kotel", ILI9341_RED, FONT_SIZE, 0, STATUS_H + 1 + 10);
  DispMsg("Обратка", txtс, FONT_SIZE + 1, (TEXT_W * 2) * 8, STATUS_H + 1);
  DispMsg("Дома", txtс, FONT_SIZE + 1, 0, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) * 3);
  DispMsg("Улица", txtс, FONT_SIZE + 1, (TEXT_W * 2) * 8, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) * 3);
  display.setFont();
}

float tpod = 0;
float tobr = 0;
float tin = 0;
float tout = 0;
byte Last_temp_in_target = 0;
byte Last_t_boiler = 0;

void DispTemp(bool wake) {
  display.setFont(&SegmentFont);
  uint16_t temp_colour;
  float cur_temp = 0;
  cur_temp = Podacha.getTemp();
  if (tpod != cur_temp or wake) {
    temp_colour = txtс;
    tpod = cur_temp;
    if (cur_temp <= cfg_t_min_pod) temp_colour = ILI9341_BLUE;
    else if (cur_temp > t_boiler) temp_colour = ILI9341_ORANGE;
    else if (cur_temp >= cfg_t_hot) temp_colour = ILI9341_RED;
    if (cur_temp == - 2000) temp_colour = ILI9341_RED;
    display.fillRect(0, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) + 3, CUSTOM_TEXT_W * 6, CUSTOM_TEXT_H, bg);
    DispMsg((cur_temp == -2000)? "//" : String(cur_temp, 1) + ",", temp_colour, 1, 0, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) + CUSTOM_TEXT_H);
  }  
  cur_temp = Obratka.getTemp();
  if (tobr != cur_temp or wake) {
    temp_colour = txtс;
    tobr = cur_temp;
    if (cur_temp < cfg_t_min_obr) temp_colour = ILI9341_BLUE;
    if (cur_temp == - 2000) temp_colour = ILI9341_RED;
    display.fillRect((TEXT_W * 2) * 8, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) + 3, CUSTOM_TEXT_W * 6, CUSTOM_TEXT_H, bg);
    DispMsg((cur_temp == -2000)? "//" : String(cur_temp, 1) + ",", temp_colour, 1, (TEXT_W * 2) * 8, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) + CUSTOM_TEXT_H);
  }
  cur_temp = In.getTemp();
  if (tin != cur_temp or wake) {
    temp_colour = txtс;
    tin = cur_temp;
    if (cur_temp > temp_in_target) temp_colour = ILI9341_ORANGE;
    else if (cur_temp < temp_in_target - 1) temp_colour = ILI9341_BLUE;
    if (cur_temp == - 2000) temp_colour = ILI9341_RED;
    display.fillRect(0, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) * 4 + 3, CUSTOM_TEXT_W * 6, CUSTOM_TEXT_H, bg);
    DispMsg((cur_temp == -2000)? "//" : String(cur_temp, 1) + ",", temp_colour, 1, 0, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) * 4 + CUSTOM_TEXT_H);
  }
  cur_temp = Out.getTemp();
  if (tout != cur_temp or wake) {
    temp_colour = txtс;
    tout = cur_temp;
    if (cur_temp == - 2000) temp_colour = ILI9341_RED;
    display.fillRect((TEXT_W * 2) * 8, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) * 4 + 3, CUSTOM_TEXT_W * 6, CUSTOM_TEXT_H, bg);
    DispMsg((cur_temp == -2000)? "//" : String(cur_temp, 1) + ",", temp_colour, 1, (TEXT_W * 2) * 8, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) * 4 + CUSTOM_TEXT_H);
  }  
  display.setFont();
  display.setTextSize(2);

  if (Last_temp_in_target != temp_in_target or wake) {
    DispMsg(String(temp_in_target), ILI9341_WHITE, 1, 0, STATUS_H + 1 + ((FONT_SIZE + 1) * 9) + 3 + CUSTOM_TEXT_H);
    Last_temp_in_target = temp_in_target;
  }
  if (Last_t_boiler != t_boiler or wake) {
    DispMsg(String(t_boiler), ILI9341_WHITE, 1, 0, STATUS_H + 1 + ((FONT_SIZE + 1) * 8) * 4 + 3 + CUSTOM_TEXT_H);  
    Last_t_boiler = t_boiler;
  }
} 

void lcdWake() {
  if (!lcdOn) {
    msshut = millis();
    ClrStatus(0,0);
    dspTime();
    drawNTP(ntpSync);
    DispTempLabel();
    DispTemp(true);
    
    DispMsg("TEN ON" + String(tenLvl), ILI9341_GREEN, 1, 0, STATUS_H + 2 + (TEXT_H * 2) * 4 + TEXT_H * 2);
    DispMsg("NASOS:" + String(speedNasos), ILI9341_GREEN, 1, (TEXT_W * 2) * 8, STATUS_H + 2 + (TEXT_H * 2) * 4 + TEXT_H * 2);
    
    bitSet(hc74, LCD_LED);
    writeHC74(hc74);
    lcdOn=true;
  }  
  else 
    msshut = millis();
}
void lcdShut() {
  display.fillRect(0, 0, WIDTH, HEIGH, bg); 
  bitClear(hc74, LCD_LED);
  writeHC74(hc74);
  lcdOn=false;
}
//---------------------------Управление выключением дисплея---------------------------
void LcdShutLoop() {
   if (cur_ms - msshut > 300000 && lcdOn ) lcdShut();
}
const uint8_t WiFiSignal[] = {
  0x00, 0x02,
  0x00, 0x02,
  0x00, 0x0A,
  0x00, 0x0A,
  0x00, 0x2A,
  0x00, 0x2A,
  0x00, 0xAA,
  0x00, 0xAA,
  0x02, 0xAA,
  0x02, 0xAA,
  0x0A, 0xAA,
  0x0A, 0xAA,
  0x2A, 0xAA,
  0x2A, 0xAA,
  0xAA, 0xAA,
  0xAA, 0xAA
}; 

int lastRssi = 0;
void drawWifi(int rssi) {
  word mult = 0;
  if (rssi >= -90) mult |= (0x02 << 14);
  if (rssi >= -80) mult |= (0x02 << 12);
  if (rssi >= -70) mult |= (0x02 << 10);
  if (rssi >= -67) mult |= (0x02 << 8);
  if (rssi >= -60) mult |= (0x02 << 6);
  if (rssi >= -50) mult |= (0x02 << 4);
  if (rssi >= -40) mult |= (0x02 << 2);
  if (rssi >= -30) mult |= 0x02;
  uint8_t pic[32];
  for (byte i = 0; i < 16; i++) {
      pic[i * 2] = WiFiSignal[i * 2] & highByte(mult);
      pic[i * 2 + 1] = WiFiSignal[i * 2 + 1] & lowByte(mult);
  }   
  display.drawBitmap(WIDTH - 16, 0, pic, 16, 16, ILI9341_BLUE);
}

const uint8_t NTPpic[] PROGMEM = {
//dot
/*| 8 4 2 1 8 4 2 1 8 4 2 1 8 4 2 1 |*/
/*| . . . . X X X X X X X X . . . . |*/  0x07,0xe0,
/*| . . X X X . . . . . . X X X . . |*/  0x1c,0x38,
/*| . X X . . . . X X . . . . X X . |*/  0x31,0x8c,
/*| . X . . . . . X X . . . . . X . |*/  0x61,0x86,
/*| X X . . . . . X X . . . . . X X |*/  0x41,0x82,
/*| X . . . . . . X X . . . . . . X |*/  0xc1,0x83,
/*| X . . . . . . X X . . . . . . X |*/  0x81,0x81,
/*| X . X . . . . X X . . . . X . X |*/  0xa1,0x85,
/*| X . X . . . . . X . . . . X . X |*/  0xa0,0xe5,
/*| X . . . . . . . . X . . . . . X |*/  0x80,0x31,
/*| X . . . . . . . . . X . . . . X |*/  0xc0,0x03,
/*| X X . . . . . . . . . X . . X X |*/  0x40,0x02,
/*| . X . . . . . . . . . . . . X . |*/  0x60,0x06,
/*| . X X . . . . X X . . . . X X . |*/  0x31,0x8c,
/*| . . X X X . . . . . . X X X . . |*/  0x1c,0x38,
/*| . . . . X X X X X X X X . . . . |*/  0x01,0xc0
};

void drawNTP(bool sync) {
  if (sync) display.drawBitmap(WIDTH - 40, 0, NTPpic, 16, 16, ILI9341_BLUE);
  else display.drawBitmap(WIDTH - 40, 0, NTPpic, 16, 16, ILI9341_RED);
}

byte scroll = false;
#define dspLines 14
String dspLog[dspLines];

void dspLogOut(String str ) {
  display.setCursor(0, STATUS_H + 1);
  display.setTextColor(ILI9341_GREEN, bg);
  display.setTextSize(FONT_SIZE);
  str = utf8rus(str);
  if (logLine > dspLines - 1) {
    logLine = 0;
    if (!scroll) scroll = true;
  }
  int lenA = dspLog[(logLine > dspLines - 1)? logLine - dspLines: logLine].length();
  int lenB;
  bool twoLines = false;
  if (str.length() > 27)  {
      dspLog[logLine] = str.substring(0,26);
      logLine++;
      twoLines = true;
      if (logLine > dspLines - 1) {
        logLine = 0;
        if (!scroll) scroll = true;
      }
      lenB = dspLog[(logLine > dspLines - 1)? logLine - dspLines: logLine].length();
      dspLog[logLine] = str.substring(26,53);
  }
  else dspLog[logLine] = str;
  int a;
  for (byte count = 0; count < dspLines; count++) {
    if (scroll) {
      int len;
      // Выводим сначала следующее сообщение (точнее последнее)
      // Чтобы последнее было наверху, далее идет предпоследнее и так далее, внизу будет текущее
      a = count + logLine + 1; 
      if (twoLines) {
        if (count == 0)
          len = lenA - dspLog[(a > dspLines - 1)? a - dspLines: a].length();
        if (count == 1)
          len = lenB - dspLog[(a > dspLines - 1)? a - dspLines: a].length();
        else 
          len = dspLog[(a - 2 > dspLines - 1)? a - 2 - dspLines: a - 2].length() - dspLog[(a > dspLines - 1)? a - dspLines: a].length(); 
        if (len > 0) {
          display.fillRect(dspLog[(a > dspLines - 1)? a - dspLines: a].length() * TEXT_W, count * TEXT_H + STATUS_H + 1, len * TEXT_W, TEXT_H, bg);
        }         
      }
      else {
        if (count == 0)
          len = lenA - dspLog[(a > dspLines - 1)? a - dspLines: a].length();
        else
          len = dspLog[(a - 1 > dspLines - 1)? a - 1 - dspLines: a - 1].length() - dspLog[(a > dspLines - 1)? a - dspLines: a].length(); 
        if (len > 0) {
          display.fillRect(dspLog[(a > dspLines - 1)? a - (dspLines): a].length() * TEXT_W, count * TEXT_H + STATUS_H + 1, len * TEXT_W, TEXT_H, bg);
        }  
      }
    }
    else
      a = count;
    display.println(dspLog[(a > dspLines - 1)? a - (dspLines): a]);    
  }
  delay(100);
  logLine++;
}

String lastTime = "09:00";
void dspTime() {
  if (GetTime().substring(0, 5) != lastTime) {
    int16_t  x1, y1;
    uint16_t w, h;
    display.setFont(&FreeSerif9pt7b);
    display.getTextBounds("09:00", 0, STATUS_H, &x1, &y1, &w, &h);  
    lastTime = GetTime().substring(0, 5);
    display.fillRect(WIDTH/2 - w/2 - x1, 0, w, STATUS_H, status_bg);
    display.getTextBounds(lastTime, 0, STATUS_H, &x1, &y1, &w, &h);
    display.setCursor(WIDTH/2 - w/2 - x1, STATUS_H - 3);
    display.setTextColor(ILI9341_BLUE, status_bg);
    display.setTextSize(1);
    display.println(lastTime);
    display.setFont();
  }
}

unsigned long lastTickDisp = 0;
void ScreenLoop() {
  LcdShutLoop();
  if ((millis() - lastTickDisp >= 1000) and lcdOn) {
    lastTickDisp = millis();
    dspTime();
    int rssi = WiFi.RSSI();
    if (rssi != lastRssi) {
      lastRssi = rssi;
      drawWifi(rssi);
    }
    DispTemp(false);

    DispMsg("Насос: "+String(speedNasos), ILI9341_GREEN, 1, (TEXT_W*2)*8, STATUS_H+2+(TEXT_H*2)*4+TEXT_H*2);
    
    display.setFont(&FreeSerif9pt7b);
    DispMsg("Тен ВКЛ: "+String(tenLvl), ILI9341_GREEN, 1, 0, STATUS_H+2+(TEXT_H*2)*4+TEXT_H*2);
    display.setFont();
  }
}
