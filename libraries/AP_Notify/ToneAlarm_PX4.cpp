
/****
 *
 *  ToneAlarm PX4 driver
 *
 ****/

/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <AP_HAL/AP_HAL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_PX4
#include "ToneAlarm_PX4.h"
#include "AP_Notify.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <drivers/drv_tone_alarm.h>
#include <stdio.h>
#include <errno.h>

// Lauren
#include "PHL_Notify.h"
#include "../AP_Param/AP_Param.h"
#include <string.h>


extern const AP_HAL::HAL& hal;

const ToneAlarm_PX4::Tone ToneAlarm_PX4::_tones[] {
    #define AP_NOTIFY_PX4_TONE_QUIET_NEG_FEEDBACK 0
    { "MFT200L4<<<B#A#2", false },
    #define AP_NOTIFY_PX4_TONE_LOUD_NEG_FEEDBACK 1
    { "MFT100L4>B#A#2P8B#A#2", false },
    #define AP_NOTIFY_PX4_TONE_QUIET_NEU_FEEDBACK 2
    { "MFT200L4<B#", false },
    #define AP_NOTIFY_PX4_TONE_LOUD_NEU_FEEDBACK 3
    { "MFT100L4>B#", false },
    #define AP_NOTIFY_PX4_TONE_QUIET_POS_FEEDBACK 4
    { "MFT200L4<A#B#", false },
    #define AP_NOTIFY_PX4_TONE_LOUD_POS_FEEDBACK 5
    { "MFT100L4>A#B#", false },
    #define AP_NOTIFY_PX4_TONE_LOUD_READY_OR_FINISHED 6
    { "MFT100L4>G#6A#6B#4", false },
    #define AP_NOTIFY_PX4_TONE_QUIET_READY_OR_FINISHED 7
    { "MFT200L4<G#6A#6B#4", false },
    #define AP_NOTIFY_PX4_TONE_LOUD_ATTENTION_NEEDED 8
    { "MFT100L4>B#B#B#B#", false },
    #define AP_NOTIFY_PX4_TONE_QUIET_ARMING_WARNING 9
    { "MNT75L1O2G", false },
    #define AP_NOTIFY_PX4_TONE_LOUD_WP_COMPLETE 10
    { "MFT200L8G>C3", false },
    #define AP_NOTIFY_PX4_TONE_LOUD_LAND_WARNING_CTS 11
    { "MBT200L2A-G-A-G-A-G-", true },
    #define AP_NOTIFY_PX4_TONE_LOUD_VEHICLE_LOST_CTS 12
    { "MBT200>B#1", true },
    #define AP_NOTIFY_PX4_TONE_LOUD_BATTERY_ALERT_CTS 13
    { "MBNT255>B#8B#8B#8B#8B#8B#8B#8B#8B#8B#8B#8B#8B#8B#8B#8B#8", true },
    #define AP_NOTIFY_PX4_TONE_QUIET_COMPASS_CALIBRATING_CTS 14
    { "MBNT255<C16P2", true },
};

bool ToneAlarm_PX4::init()
{
   // Lauren: start

   enum ap_var_type var_type;
   int paramval;
   AP_Param *vp;

   // audio device type 
   vp = AP_Param::find("EPM_REGRAB", &var_type);
   if (vp == NULL) 
      phlaudiodisabled = true;

   paramval = int(vp->cast_to_float(var_type));
   switch (paramval) {
      case 15: 
         passive = true;  // passive device
         break;
      case 30:
         passive = false; // active device
         break;
      default:
         phlaudiodisabled = true; // invalid device setting, audio disabled
   }

   // output pin number
   vp = AP_Param::find("EPM_GRAB", &var_type);
   if (vp == NULL) {
      phlaudiodisabled = true;
   }

   paramval = int(vp->cast_to_float(var_type));
   switch (paramval) {
      case 1100:
         outputpin = AUX1;
         break;
      case 1200:
         outputpin = AUX2;
         break;
      case 1300:
         outputpin = AUX3;
         break;
      case 1400:
         outputpin = AUX4;
         break;
      case 1500:
         outputpin = AUX5;
         break;
      case 1600:
         outputpin = AUX6;
         break;
      default:
        phlaudiodisabled = true;  // invalid pin setting, audio disabled
    }

   // full signals?
   vp = AP_Param::find("EPM_NEUTRAL", &var_type);
   if (vp == NULL) {
      phlaudiodisabled = true;
   }

   paramval = int(vp->cast_to_float(var_type));
   switch (paramval) {
      case 1500: 
         fullsignals = true; // fullsignals
         break;
      case 1600:
         fullsignals = false; // subset of signals
         break;
      case 1999:
         tunestest = true;   // tunes test
         break;  
      default:
         phlaudiodisabled = true; // invalid setting, audio disabled
   }

   // Logic level for device OFF
   vp = AP_Param::find("EPM_RELEASE", &var_type);
   if (vp == NULL) {
      phlaudiodisabled = true;
   }

   paramval = int(vp->cast_to_float(var_type));
   switch (paramval) {
      case 1500:
          lowoff = true; // logic level low for OFF
          hal.gpio->pinMode(outputpin, HAL_GPIO_OUTPUT);
          hal.gpio->write(outputpin, 0);
          break;
      case 1600:
          lowoff = false; // logic level high for OFF
          hal.gpio->pinMode(outputpin, HAL_GPIO_OUTPUT);
          hal.gpio->write(outputpin, 1);
          break;
      default:
          phlaudiodisabled = true;  // invalid logic level OFF setting,
				    // audio disabled
    }

    phltones(TUNE_INIT); // play init audio sequence

    // Lauren: Original Pixhawk audio code continues below
    
    // open the tone alarm device
    _tonealarm_fd = open(TONEALARM0_DEVICE_PATH, O_WRONLY);
    if (_tonealarm_fd == -1) {
        hal.console->printf("ToneAlarm_PX4: Unable to open " TONEALARM0_DEVICE_PATH);
        return false;
    }
    
    // set initial boot states. This prevents us issuing a arming
    // warning in plane and rover on every boot
    flags.armed = AP_Notify::flags.armed;
    flags.failsafe_battery = AP_Notify::flags.failsafe_battery;
    flags.pre_arm_check = 1;
    _cont_tone_playing = -1;
    return true;
}

// play_tune - play one of the pre-defined tunes
void ToneAlarm_PX4::play_tone(const uint8_t tone_index)
{
    // Lauren
    if (!phlaudiodisabled)  // PHL audio enabled?
       return;		    // yes, don't try play standard audio

    uint32_t tnow_ms = hal.scheduler->millis();
    const Tone &tone_requested = _tones[tone_index];

    if(tone_requested.continuous) {
        _cont_tone_playing = tone_index;
    }

    _tone_playing = tone_index;
    _tone_beginning_ms = tnow_ms;

    play_string(tone_requested.str);
}

void ToneAlarm_PX4::play_string(const char *str) {
    write(_tonealarm_fd, str, strlen(str) + 1);
}

void ToneAlarm_PX4::stop_cont_tone() {
    // Lauren
    if (!phlaudiodisabled)  // PHL audio enabled?
       return;		    // yes, not playing standard audio

    if(_cont_tone_playing == _tone_playing) {
        play_string("");
        _tone_playing = -1;
    }
   _cont_tone_playing = -1;
}

void ToneAlarm_PX4::check_cont_tone() {
    if (!phlaudiodisabled)  // PHL audio enabled?
       return;		    // yes, not playing standard audio

    uint32_t tnow_ms = hal.scheduler->millis();
    // if we are supposed to be playing a continuous tone,
    // and it was interrupted, and the interrupting tone has timed out,
    // resume the continuous tone

    if (_cont_tone_playing != -1 && _tone_playing != _cont_tone_playing && tnow_ms-_tone_beginning_ms > AP_NOTIFY_PX4_MAX_TONE_LENGTH_MS) {
        play_tone(_cont_tone_playing);
    }
}

// update - updates led according to timed_updated.  Should be called at 50Hz
void ToneAlarm_PX4::update()
{
    // exit immediately if we haven't initialised successfully
    if (_tonealarm_fd == -1) {
        return;
    }

    check_cont_tone();

    if (AP_Notify::flags.compass_cal_running != flags.compass_cal_running) {
        if(AP_Notify::flags.compass_cal_running) {
            play_tone(AP_NOTIFY_PX4_TONE_QUIET_COMPASS_CALIBRATING_CTS);
            play_tone(AP_NOTIFY_PX4_TONE_QUIET_POS_FEEDBACK);
        } else {
	    if (!phlaudiodisabled)  // PHL audio enabled?
       	       return;		    // yes, not playing standard audio
            if(_cont_tone_playing == AP_NOTIFY_PX4_TONE_QUIET_COMPASS_CALIBRATING_CTS) {
                stop_cont_tone();
            }
        }
    }
    flags.compass_cal_running = AP_Notify::flags.compass_cal_running;

    if (AP_Notify::events.compass_cal_canceled) {
        play_tone(AP_NOTIFY_PX4_TONE_QUIET_NEU_FEEDBACK);
        return;
    }

    if (AP_Notify::events.initiated_compass_cal) {
        play_tone(AP_NOTIFY_PX4_TONE_QUIET_NEU_FEEDBACK);
        return;
    }

    if (AP_Notify::events.compass_cal_saved) {
        play_tone(AP_NOTIFY_PX4_TONE_QUIET_READY_OR_FINISHED);
        return;
    }

    if (AP_Notify::events.compass_cal_failed) {
        play_tone(AP_NOTIFY_PX4_TONE_QUIET_NEG_FEEDBACK);
        return;
    }

    // don't play other tones if compass cal is running
    if (AP_Notify::flags.compass_cal_running) {
        return;
    }

    // notify the user when autotune or mission completes
    if (AP_Notify::flags.armed && (AP_Notify::events.autotune_complete || AP_Notify::events.mission_complete)) {
        play_tone(AP_NOTIFY_PX4_TONE_LOUD_READY_OR_FINISHED);
    }

    //notify the user when autotune fails
    if (AP_Notify::flags.armed && (AP_Notify::events.autotune_failed)) {
        play_tone(AP_NOTIFY_PX4_TONE_LOUD_NEG_FEEDBACK);
    }

    // notify the user when a waypoint completes
    if (AP_Notify::events.waypoint_complete) {
        play_tone(AP_NOTIFY_PX4_TONE_LOUD_WP_COMPLETE);
    }

    // notify the user when their mode change was successful
    if (AP_Notify::events.user_mode_change) {
        if (AP_Notify::flags.armed) {
            play_tone(AP_NOTIFY_PX4_TONE_LOUD_NEU_FEEDBACK);
        } else {
            play_tone(AP_NOTIFY_PX4_TONE_QUIET_NEU_FEEDBACK);
        }
    }

    // notify the user when their mode change failed
    if (AP_Notify::events.user_mode_change_failed) {
        if (AP_Notify::flags.armed) {
            play_tone(AP_NOTIFY_PX4_TONE_LOUD_NEG_FEEDBACK);
        } else {
            play_tone(AP_NOTIFY_PX4_TONE_QUIET_NEG_FEEDBACK);
        }
    }

    // failsafe initiated mode change
    if(AP_Notify::events.failsafe_mode_change) {
       if (fullsignals)
          phltones(TUNE_RC_EVENT); // Lauren
       play_tone(AP_NOTIFY_PX4_TONE_LOUD_ATTENTION_NEEDED);
    }

    // notify the user when arming fails
    if (AP_Notify::events.arming_failed) {
       if (fullsignals)
	   phltones(TUNE_ARMFAIL); // Lauren
        play_tone(AP_NOTIFY_PX4_TONE_QUIET_NEG_FEEDBACK);
    }

    // notify the user when RC contact is lost
    if (flags.failsafe_radio != AP_Notify::flags.failsafe_radio) {
        flags.failsafe_radio = AP_Notify::flags.failsafe_radio;
        if (flags.failsafe_radio) {
            // armed case handled by events.failsafe_mode_change
            if (!AP_Notify::flags.armed) {
	       if (fullsignals)
	          phltones(TUNE_RC_EVENT); // Lauren
               play_tone(AP_NOTIFY_PX4_TONE_QUIET_NEG_FEEDBACK);
            }
        } else {
            if (AP_Notify::flags.armed) {
	       if (fullsignals)
	          phltones(TUNE_RC_EVENT); // Lauren
               play_tone(AP_NOTIFY_PX4_TONE_LOUD_POS_FEEDBACK);
            } else {
	       if (fullsignals)
	          phltones(TUNE_RC_EVENT); // Lauren
               play_tone(AP_NOTIFY_PX4_TONE_QUIET_POS_FEEDBACK);
            }
        }
    }

    // notify the user when pre_arm checks are passing
    if (flags.pre_arm_check != AP_Notify::flags.pre_arm_check) {
        flags.pre_arm_check = AP_Notify::flags.pre_arm_check;
        if (flags.pre_arm_check) {
	    if (fullsignals)
	       phltones(TUNE_PREARMED); // Lauren
            play_tone(AP_NOTIFY_PX4_TONE_QUIET_READY_OR_FINISHED);
        }
    }

    // check if arming status has changed
    if (flags.armed != AP_Notify::flags.armed) {
        flags.armed = AP_Notify::flags.armed;
        if (flags.armed) {
            // arming tune
	    phltones(TUNE_ARMED); // Lauren
            play_tone(AP_NOTIFY_PX4_TONE_QUIET_ARMING_WARNING);
        } else {
            // disarming tune
	    phltones(TUNE_DISARMED); // Lauren
            if (phlaudiodisabled) {  // PHL audio not enabled?
	       play_tone(AP_NOTIFY_PX4_TONE_QUIET_NEU_FEEDBACK);
               stop_cont_tone();
            }
        }
    }

    // check if battery status has changed
    if (flags.failsafe_battery != AP_Notify::flags.failsafe_battery) {
        flags.failsafe_battery = AP_Notify::flags.failsafe_battery;
        if (flags.failsafe_battery) {
            // battery warning tune
            play_tone(AP_NOTIFY_PX4_TONE_LOUD_BATTERY_ALERT_CTS);
        }
    }

    // Lauren
    // Check if gps status has changed
    static int gpsprevtune = 0;  // previous PHL melody signal for gps
    if (flags.gps_status != AP_Notify::flags.gps_status) {
        flags.gps_status = AP_Notify::flags.gps_status;
        switch (flags.gps_status) {
           case 1: // no lock
	      if (gpsinit == true) { // don't play NOLOCK for gps init 
                 gpsinit = false;
                 break;
              }   
	      if (fullsignals || (!fullsignals && !gpsprevtune == 2))
   	         phltones(TUNE_GPS_NOLOCK); 
	      gpsprevtune = 1; // the new previous tune
	      break;
           case 2: // 2d lock
	      if (fullsignals)
                 phltones(TUNE_GPS_2D);
	      else if (gpsprevtune == 3 || gpsprevtune == 4)
		 phltones(TUNE_GPS_NOLOCK);
	      gpsprevtune = 2;    	
              break;
           case 3: // 3d lock
	      if (fullsignals || (!fullsignals && gpsprevtune != 4))
                 phltones(TUNE_GPS_3D);
              gpsprevtune = 3;
              break;
           case 4: // dgps lock
	      if (fullsignals 
                    && (!fullsignals && gpsprevtune != 3))
                 phltones(TUNE_GPS_DGPS);
	      gpsprevtune = 4;
              break;
           default:
              break;
        }
    }

    // check parachute release
    if (flags.parachute_release != AP_Notify::flags.parachute_release) {
        flags.parachute_release = AP_Notify::flags.parachute_release;
        if (flags.parachute_release) {
            // parachute release warning tune
            play_tone(AP_NOTIFY_PX4_TONE_LOUD_ATTENTION_NEEDED);
        }
    }

    // lost vehicle tone
    if (flags.vehicle_lost != AP_Notify::flags.vehicle_lost) {
        flags.vehicle_lost = AP_Notify::flags.vehicle_lost;
        if (flags.vehicle_lost) {
           if (tunestest) { // Play all tones test sequence (for current device mode)
              for (int i = 1; i <= TUNE_LOST; ++i) {
                  phltones(i);
                  // hal.scheduler->delay(5000); // sleep 5 secs
              }
              return;
            }
            phltones(TUNE_LOST); // Lauren
            play_tone(AP_NOTIFY_PX4_TONE_LOUD_VEHICLE_LOST_CTS);
        } else {
            if (phlaudiodisabled)   // PHL audio not enabled?
               stop_cont_tone();
        }
    }
}

// Lauren
void ToneAlarm_PX4::phltones(int tune) {
   int currnote = 0;
   int beat = 0;
   char *currtune, *savetune;
   char *p, *p1;
   long duration;

   // no audio if disabled or possibly in flight (currently armed)
   if (phlaudiodisabled == true || (flags.armed && (tune != TUNE_ARMED)))
      return; /* no audio */   

//   if (tune == lasttune)  // don't play same melody twice in a row
//      return; /* no audio */

   // proceed with audio

   lasttune = tune;  /* to avoid playing same melody twice in sequence */

   switch (tune) {
      case TUNE_INIT:
         currtune = (passive ? tune_init_p : tune_init_a);          
         break;
      case TUNE_ARMFAIL:
         currtune = (passive ? tune_armfail_p : tune_armfail_a);
         break;
      case TUNE_PREARMED:
         currtune = (passive ? tune_prearmok_p : tune_prearmok_a);
         break;
      case TUNE_ARMED:
         currtune = (passive ? tune_armed_p : tune_armed_a);
         break;
      case TUNE_DISARMED:
         currtune = (passive ? tune_disarmed_p : tune_disarmed_a);
         break;
      case TUNE_GPS_2D:
         currtune = (passive ? tune_gps_2d_p : tune_gps_2d_a);
         break;
      case TUNE_GPS_3D:
         currtune = (passive ? tune_gps_3d_p : tune_gps_3d_a);
         break;
      case TUNE_GPS_DGPS:
         currtune = (passive ? tune_gps_dgps_p : tune_gps_dgps_a);
         break;
      case TUNE_GPS_NOLOCK:   
         currtune = (passive ? tune_gps_nolock_p : tune_gps_nolock_a);
         break;
      case TUNE_RC_EVENT:   
         currtune = (passive ? tune_rc_event_p : tune_rc_event_a);
         break;
      case TUNE_LOST:   
         currtune = (passive ? tune_lost_p : tune_lost_a);
         break;
      default:
	 return;
   }

   if ((savetune = strdup(currtune)) == NULL) // strtok is destructive 
      return; // trouble duping string

   p = strtok(currtune, " ,"); // deliminators are space or comma 
   while (p != NULL) {          // until melody null termination 
      // first the note designation
      if (*p >= 'a' && *p <= 'g') // note
         currnote = notes[(int)*p - NOTEOFFSET1];
      else if (*p >= 'A' && *p <= 'G') // note
         currnote = notes[(int)*p - NOTEOFFSET2];
      else if (*p == 'P') // pause
         currnote = 0;
      else {
         free(savetune);
         return; // bad format, no audio to play
      }

      // check for first numeric beat digit (1->9)
      p1 = p+1;
      if (*p1 < '1' || *p1 > '9') { // check for numeric beat digits
         free(savetune);
         return; // bad melody format, don't try play
      }

      beat = atoi(p1); // get the beat!
      p = strtok(NULL, " ,"); // set up next token (if any)
       
      duration = beat * tempo; 
      long elapsed_time = 0;

      if (currnote) {  // not pause?
         hal.gpio->pinMode(outputpin, HAL_GPIO_OUTPUT);
         while (elapsed_time < duration) {
            // UP
            hal.gpio->write(outputpin, 1);
            hal.scheduler->delay_microseconds(currnote/2);
 
            // DOWN
            hal.gpio->write(outputpin, 0);
            hal.scheduler->delay_microseconds(currnote/2);

            // Keep track of how long we pulsed
            elapsed_time += (currnote);
        }

      }
      else { // pause 
         hal.gpio->pinMode(AUX1, HAL_GPIO_INPUT);
         for (int j = 0; j < pausecount; j++) 
            hal.scheduler->delay_microseconds(duration);
      }     

      // silence between notes
      if (lowoff == false)
         hal.gpio->pinMode(outputpin, HAL_GPIO_INPUT);
      hal.scheduler->delay_microseconds(silnotes);

      // continuous tune play (lost tune) checks start here

      if (p == NULL) {  // reached end of tune?
	 strcpy(currtune, savetune);
	 free(savetune);

         if (tune == TUNE_LOST) { // lost tune plays continuously until reboot
            if ((savetune = strdup(currtune)) == NULL) 
               return; 
            p = strtok(currtune, " ,"); // deliminators are space or comma 
         }
      }

      // continuous tune play checks end here
   }

   // end of tune

   if (lowoff == false) 
       hal.gpio->pinMode(outputpin, HAL_GPIO_INPUT);
   else { // lowoff = true
       hal.gpio->pinMode(outputpin, HAL_GPIO_OUTPUT);
       hal.gpio->write(outputpin, 0);
   }

   hal.scheduler->delay(2000); // delay 2 seconds after playing tune
}


#endif // CONFIG_HAL_BOARD == HAL_BOARD_PX4
