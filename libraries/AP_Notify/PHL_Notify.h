
//
// Tones and buzzer output for Pixhawk Lite 
//
// Lauren Weinstein (lauren@vortex.com, http://lauren.vortex.com)
// 11/2015
//

#include "PHL_Tunes.h"

// Audio device settings, via usually 
// available "borrowed" parameters 

// Audio device type (e.g. piezo)
// Passive devices can play melodies, active are fixed frequency buzzers
#define PHL_TYPE "EPM_REGRAB" // 0=audio disabled,
                              // 15=passive device, 30=active device (buzzer)
			      // Any other values disable audio
			      // Default=0 (audio disabled)

// AUX output pin for audio device (AUX1 through 6 inclusive to enable audio)
#define PHL_PIN  "EPM_GRAB" // 1100=AUX1, 1200=AUX2, 1300=AUX3, 1400=AUX4,
                            // 1500=AUX5, 1600=AUX6
                            // Any other values disable audio
			    // Default=1900 (audio disabled)

// Logic level for device OFF (does logic low or high silence the device?)
#define PHL_OFF_LEVEL  "EPM_RELEASE" // 1500=silent at logic low
                                     // 1600=silent at logic high
				     // Any other values disable audio
                                     // Default=1100 (audio disabled)

// Select full melody/pattern signals, or only a subset
#define PHL_FULL  "EPM_NEUTRAL"      // 1500=full signals
                                     // 1600=subset of signals
                                     // 1999=tunes test			
				     // Any other values disable audio
                                     // Default=1500 (full signals)

/* Pixhawk Lite AUX output pins (1-6) */
#define AUX1 50
#define AUX2 51
#define AUX3 52
#define AUX4 53
#define AUX5 54
#define AUX6 55

long tempo  		= 50000;   // overall tempo 
long silnotes	        = 100000;  // silence between notes 
int pausecount          = 10;      // adjustment for pause length 
bool passive            = false;   // true for passive device, false for active device (buzzer)
bool lowoff             = false;   // logic level OFF (true for low off, false for high off)
bool gpsinit	        = true;    // true for gps initialization, false afterwards
bool fullsignals        = false;   // true for full signals, false for partial signals
bool phlaudiodisabled   = false;   // true for audio disabled 
bool tunestest          = false;   // true for tunes test
int outputpin           = 0;       // audio output pin (AUX1 through AUX6) 
int lasttune	        = 0;       // most recent melody played


