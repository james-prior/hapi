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

void setupControls(void) {
  int i;
  // Initialize
}

void poll_timed_things(void) {
  currentTime = now();

  for (int i = 0; i < ARRAY_LENGTH(c_functions); i++)
    c_functions[i].poll_time_thing_function(i);
}

float poll_timed_thing(int i) {
  CFuncDef c;
  ControlData d;
  c = c_functions[i];
  d = c_data[i];

  if (d.hc_active) {                  // Is the thing on?
    if (d.hc_end > currentTime) {     // Yes, should it be turned off?
      d.hc_active = false;
      digitalWrite(d.hc_controlpin, !d.hc_polarity);
      if (d.hc_repeat != 0) {   // Is repeat active?
        d.hc_start += d.hc_repeat;
        d.hc_end += d.hc_repeat;
      }
    }
    if (c.iPtr(i) < d.hcs_offValue) { // Is the TurnOff value exceeded?
      d.hc_active = false;
      digitalWrite(d.hc_controlpin, !d.hc_polarity);
    }
  } else if (d.hc_start >= currentTime || c.iPtr(i) > d.hcs_onValue) { // Is the turnOn value exceeded?
    d.hc_active = true;        // Turn it On, thing is now on
    digitalWrite(d.hc_controlpin, d.hc_polarity);
  }
}
