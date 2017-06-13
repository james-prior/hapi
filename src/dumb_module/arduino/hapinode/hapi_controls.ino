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

HAPI Remote Terminal Unit Firmware Code v3.0.0
Authors: Tyler Reed, Mark Miller
ESP Modification: John Archbold

Sketch Date: May 2nd 2017
Sketch Version: v3.0.0
Implements definitions for
  ESP-NodeMCU
  ESP8266
  WROOM32
*/

void setupControls(void){
  int i;
  // Initialize
}

void checkControls(void) {
  CFuncDef c;
  currentTime = now();            // Update currentTime and ..
                                  //  check all the control functions
  for (int device=0;device<ArrayLength(HapicFunctions);device++) { // For each device
    c = HapicFunctions[device];                //  initialize access structure
    c.oPtr(device);                            //  call the check function
  }
}

float poll_on_off_thing_controller(int i) {
  CFuncDef c;
  ControlData d;
  c = HapicFunctions[i];
  d = HapicData[i];
  if (d.hc_active) { // is it on?
    if (d.hc_end > currentTime) {     // Yes, should it be turned off?
      d.hc_active = false;
      digitalWrite(d.hc_controlpin, !d.hc_polarity);
      if (d.hc_repeat != 0) {   // Is repeat active?
        d.hc_start += d.hc_repeat;
        d.hc_end += d.hc_repeat;
      }
    }
    if (c.iPtr(i) < d.hcs_offValue) { // is the TurnOff value exceeded?
      d.hc_active = false;
      digitalWrite(d.hc_controlpin, !d.hc_polarity);
    }
  } else if (d.hc_start >= currentTime || c.iPtr(i) > d.hcs_onValue) {
    d.hc_active = true;        // Turn it on
    digitalWrite(d.hc_controlpin, d.hc_polarity);
  }
}

float controlPumps(int Device) {
  poll_on_off_thing_controller(Device);
}

float controlLamps(int Device) {
  poll_on_off_thing_controller(Device);
}
