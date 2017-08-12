#include <Arduino.h>

/*
#*********************************************************************
#Copyright 2016 Maya Culpa, LLC
#
#This program is free software: you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation, either version 3 of the License, or
#(at your option) any later version.
#
#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.
#
#You should have received a copy of the GNU General Public License
#along with this program.  If not, see <http://www.gnu.org/licenses/>.
#*********************************************************************
*/

#define SECONDS_PER_MINUTE (60)
#define MINUTES_PER_HOUR (60)
#define HOURS_PER_DAY (24)
#define DAYS_PER_NON_LEAP_YEAR (365)
#define SECONDS_PER_HOUR (MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
#define SECONDS_PER_DAY ((unsigned long)HOURS_PER_DAY * SECONDS_PER_HOUR)
#define N_LEAP_DAYS(year) ((year)/4 - (year)/100 + (year)/400)
// number of leap days from beginning of first_year to beginning of last_year
#define N_LEAP_DAYS_DELTA(first_year, last_year) (\
    N_LEAP_DAYS(last_year) - N_LEAP_DAYS(first_year))
#define NTP_BASE_YEAR (1900)
#define UNIX_EPOCH_BASE_YEAR (1970)
#define NTP_TO_UNIX_DAYS ( \
    (UNIX_EPOCH_BASE_YEAR - NTP_BASE_YEAR) * DAYS_PER_NON_LEAP_YEAR\
    + N_LEAP_DAYS_DELTA(NTP_BASE_YEAR, UNIX_EPOCH_BASE_YEAR))
#define NTP_TO_UNIX_SECONDS (NTP_TO_UNIX_DAYS * SECONDS_PER_DAY)

time_t getNtpTime(void)
{
  while (udp.parsePacket() > 0) // discard any previously received packets
    ;
  Serial.println(F("Transmit NTP Request"));
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println(F("Receive NTP Response"));
      udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;

  // The timestamp starts at byte 40 of the received packet and is four bytes,
  // or two words, long. First, extract the two words:
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - NTP_TO_UNIX_SECONDS + timeZone * (unsigned long)SECS_PER_HOUR;
    }
  }
  Serial.println(F("No NTP Response :-("));
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  Serial.println(F("sending NTP packet..."));
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
  udp.beginPacket(address, NTP_port);
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

