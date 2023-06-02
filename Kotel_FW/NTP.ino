#include <WiFiUdp.h>            //необходима для NTP 

bool NtpFirstTry = true;
unsigned int localPort = 2390;      // local port to listen for UDP packets
WiFiUDP NTP;
unsigned long ntpLastCheck = 0;
unsigned long epoch = 0;
byte hour, minutes, seconds;
bool sentNtpPacket = false;
unsigned int lastSendNtpPacket;

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

void NTPInit() {
  NTP.begin(localPort);
}

void NTP_loop() {
  if ((millis() - ntpLastCheck >= cfg_ntpPeriod * 1000 or sentNtpPacket or NtpFirstTry) and WiFi.status() == WL_CONNECTED) {
    //get a random server from the pool
    if (!sentNtpPacket) {
      IPAddress timeServerIP; // time.nist.gov NTP server address
      WiFi.hostByName(cfg_ntpServerName, timeServerIP);
      sendNTPpacket(timeServerIP); // send an NTP packet to a time server
      // wait to see if a reply is available
      lastSendNtpPacket = millis();
      sentNtpPacket = true;
    }
    else if (millis() - lastSendNtpPacket >= 1000) {
      sentNtpPacket = false;
      int cb = NTP.parsePacket();
      if (!cb) {
        ntpSync = false;
      }
      else {
        // We've received a packet, read the data from it
        NTP.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

        //the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, esxtract the two words:

        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);

        epoch = (highWord << 16 | lowWord) - 2208988800UL;
        ntpLastCheck = millis();
        ntpSync = true;
        if (NtpFirstTry) {
          NtpFirstTry = false;
          dspTime();
        }
      }
    }
    if (lcdOn) drawNTP(ntpSync);
  }
}

String GetTime() {
  unsigned long _epoch;
  String str;
  _epoch = epoch + (millis() - ntpLastCheck) / 1000ul;
  hour = (cfg_timezone + (_epoch / 3600ul)) % 24;
  minutes = (_epoch  % 3600ul) / 60ul;
  seconds = (_epoch % 3600ul) % 60ul;
  return timeToStr(hour, minutes, seconds);
}

String LastSync() {
  
  return  utf8rus(String(((millis() - ntpLastCheck) / 3600000 / 24) == 0 ? "Сегодня в ": String(((millis() - ntpLastCheck) / 3600000 / 24)) + " дней назад в ")) +  String((cfg_timezone + (epoch / 3600ul)) % 24) + ":" + String((epoch  % 3600ul) / 60ul) + ":" + String((epoch % 3600ul) % 60ul);
}

#define LEAP_YEAR(Y)     ( ((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && ( ((1970 + (Y)) % 100) || !((1970 + (Y)) % 400) ) )
static  const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

int GetPartOfTime(byte type) {
  uint32_t tm = (cfg_timezone + ((epoch + (millis() - ntpLastCheck) / 1000ul) / 3600ul)) / 24;
  uint8_t year = 0;
  uint8_t month, monthLength;
  unsigned long days = 0;

  while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= tm) {
    year++;
  }

  days -= LEAP_YEAR(year) ? 366 : 365;
  tm  -= days; // now it is days in this year, starting at 0


  for (month = 0; month < 12; month++) {
    if (month == 1 and LEAP_YEAR(year)) { // february
      monthLength = 29;
    } else {
      monthLength = monthDays[month];
    }

    if (tm >= monthLength) {
      tm -= monthLength;
    } else {
      break;
    }
  }
  month += 1;  // jan is month 1
  days = tm + 1;     // day of month
  year += 1987;
  switch (type) {
    case 0:
      return days;
    case 1:
      return month;
    case 2:
      return year;
    case 3: // Day Of Weak
      return ((cfg_timezone + ((epoch + (millis() - ntpLastCheck) / 1000ul) / 3600ul)) / 24 + 4) % 7;
    case 4: // HOUR
      return (cfg_timezone + ((epoch + (millis() - ntpLastCheck) / 1000ul) / 3600ul)) % 24;
  }
}

String timeToStr(int _h, int _m, int _s) {
  return ((_h > 9) ? String(_h) : "0" + String(_h)) + ":" + ((_m > 9) ? String(_m) : "0" + String(_m)) + ":" + ((_s > 9) ? String(_s) : "0" + String(_s));
}

String Uptime() {
  String str;
  byte _h, _m, _s;
  unsigned long _now = millis() / 1000;
  _h = ((_now  % 86400L) / 3600);
  _m = (_now  % 3600) / 60;
  _s = _now % 60;
  return timeToStr(_h, _m, _s);
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress& address) {
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  NTP.beginPacket(address, 123); //NTP requests are to port 123
  NTP.write(packetBuffer, NTP_PACKET_SIZE);
  NTP.endPacket();
}
