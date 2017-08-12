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

struct ntp_packet_struct {
  byte ignore1[40];
  uint32_t now; // time since 1900. Unit is 1 second. MSB first
  byte ignore2[4];
};

uint32_t ntohl(uint32_t x/*in network order*/)
{
  // network order is most significant byte first
  // https://en.wikipedia.org/wiki/Network_order#Networking
  unsigned long y = 0UL;

  for (int i = 0; i < sizeof(x); i++)
    y <<= 8;
    y |= ((byte *)&x)[i];
  return y;
}

time_t getNtpTime(void)
{
  struct ntp_packet_struct packet;
  uint32_t ntp_now; // 0 is beginning of 1900. Unit is 1 second.
  uint32_t unix_now; // 0 is beginning of 1970. Unit is 1 second.
  uint32_t local_unix_now; // 0 is beginning of 1970 local time zone. Unit is 1 second.

  while (udp.parsePacket() > 0) // discard any previously received packets
    ;
  Serial.println(F("Transmit NTP Request"));
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = udp.parsePacket();
    if (size >= sizeof(packet)) {
      Serial.println(F("Receive NTP Response"));
      udp.read(&packet, sizeof(packet));
      ntp_now = ntohl(packet.now);
      unix_now = ntp_now - NTP_TO_UNIX_SECONDS;
      local_unix_now = unix_now + timeZone * (unsigned long)SECS_PER_HOUR;
      return local_unix_now;
    }
  }
  Serial.println(F("No NTP Response :-("));
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  struct ntp_packet_struct packet;

  Serial.println(F("sending NTP packet..."));
  // set all bytes in the buffer to 0
  memset(&packet, 0, sizeof(packet));
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets) ^^^ what is that URL?
  #define packet_as_bytes ((byte *)&packet)
  packet_as_bytes[0] = 0b11100011;   // LI, Version, Mode
  packet_as_bytes[1] = 0;     // Stratum, or type of clock
  packet_as_bytes[2] = 6;     // Polling Interval
  packet_as_bytes[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packet_as_bytes[12] = 49;
  packet_as_bytes[13] = 0x4E;
  packet_as_bytes[14] = 49;
  packet_as_bytes[15] = 52;

  // send packet that requests time
  udp.beginPacket(address, NTP_port);
  udp.write(&packet, sizeof(packet));
  udp.endPacket();
}

