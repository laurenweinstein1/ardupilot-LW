
//
// Melodies/patterns for Pixhawk Lite
// Passive for passive piezo that can play various notes
// Active for active piezo (buzzer) that can only play a single fixed note
//
// Lauren Weinstein (lauren@vortex.com, http://lauren.vortex.com)
// 11/2015
//

#define TUNE_INIT 	   1    // initialize
#define TUNE_ARMFAIL  	   2    // prearm fail
#define TUNE_PREARMED 	   3    // prearmed ok
#define TUNE_ARMED         4    // armed
#define TUNE_DISARMED      5    // disarmed
#define TUNE_GPS_2D        6    // gps 2d lock
#define TUNE_GPS_3D        7    // gps 3d lock
#define TUNE_GPS_DGPS      8    // gps dgps lock
#define TUNE_GPS_NOLOCK    9    // gps loss of lock
#define TUNE_RC_EVENT	   10   // rc event
#define TUNE_LOST	   11   // lost flyer

#define NOTEOFFSET1	   97  // a->g
#define NOTEOFFSET2	   58  // A->G

// a = 4545 (220 Hz) / b = 4048 (247) / c = 3831 (261) [C4)]
// d = 3400 (294)    / e = 3038 (329) / f = 2864 (349) / g = 2550 (392)
// A = 2272 (440)    / B = 2028 (493) / C = 1912 (523) / D = 1703 (587)
// P = 0 (Pause)

const int notes[] = { 4545, 4048, 3831, 3400, 3038, 
                      2864, 2550, 2272, 2028, 1912, 
                      1703, 0 };

//
// Melodies and Patterns
//
// Passive devices can play arbitrary tones, so both the note designations
// and patterns below are relevant for them ("passive" melody arrays).
//
// Active devices (e.g. buzzers) play a single tone, so any valid
// note may be specified in their sequences -- only the patterns
// are relevant for them ("active" melody arrays).
//

// system init (passive)
char tune_init_p[] =  "d2 e2 f2 g2 A2 B2 C2 D2";
// system init (active)
char tune_init_a[] =  "P50 P50 P50 d2 P2 f2 g2 P2 B2 C2 D2 P2 a2 a2 a2 a2";
                      // p p p 2 p 2 2 p 2 2 2 p 2 2 2 2

// arming failed (passive)
char tune_armfail_p[] = "C6 P4 B6 P4 A6";
// arming failed (active)
char tune_armfail_a[] = "C8 P4 B8 P4 A8 P4 A8"; 
                         // 8 p 8 p 8 p 8

// prearm ok (passive)
char tune_prearmok_p[] = "D2 D2 D2 D2 P45 D2 D2 D2 D2";
// prearm ok (active)
char tune_prearmok_a[] = "D2 D2 D2 D2 P45 D2 D2 D2 D2"; 
                          // 2 2 2 2 p 2 2 2 2

// armed (passive)
char tune_armed_p[] = "A2 B2 C2 D2 P2 A2 B2 C2 D2";
// armed (active)
char tune_armed_a[] = "A2 B2 C2 P3 A2 B2 D2 P3 B2 C2 D2"; 
                       // 2 2 2 p 2 2 2 p 2 2 2

// disarmed (passive)
char tune_disarmed_p[] = "D5 C5 B5 A5 P2 D5 C5 B5 A5";
// disarmed (active)
char tune_disarmed_a[] = "D6 C6 B2 P1 D6 C6 B2 P1 D6 C6 B2"; 
                          // 6 6 2 p 6 6 2 p 6 6 2

// gps 2d lock (passive)
char tune_gps_2d_p[] = "C4 C4";
// gps lock (active)
char tune_gps_2d_a[] = "C4 C4"; 
                        // 4 4

// gps 3d lock (passive)
char tune_gps_3d_p[] = "C4 C4 C4";
// gps 3d lock (active)
char tune_gps_3d_a[] = "C4 C4 C4"; 
                        // 4 4 4

// gps dgps lock (passive)
char tune_gps_dgps_p[] = "C4 C4 C4 D4";
// gps dgps lock (active)
char tune_gps_dgps_a[] = "C4 C4 C4 P2 D14"; 
                          // 4 4 4 p 14

// gps no lock (passive)
char tune_gps_nolock_p[] = "B4 A4 P3 B4 A4";
// gps no lock (active)
char tune_gps_nolock_a[] = "B16 P1 A4 P1 B4 P1 A4"; 
                            // 16 p 4 p 4 p 4

// rc event (passive)
char tune_rc_event_p[] = "D4 D4 g4 g4 D4 D4";
// rc event (active)
char tune_rc_event_a[] = "D6 D6 g2 g2 D6 D6"; 
                          // 6 6 2 2 6 6

// lost vehicle (passive)
char tune_lost_p[] 
   = "a1 b1 c1 d1 e1 f1 g1 A1 B1 C1 D1 C1 B1 A1 g1 f1 e1 d1 c1 b1";
// lost vehicle (active)
char tune_lost_a[] = "a3 a6 a3 a6"; 
                      // 3 6 3 6 ...


