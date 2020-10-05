#ifndef __EDIT_WAYPOINT_SCREEN_H__
#define __EDIT_WAYPOINT_SCREEN_H__

#include "libbip.h"

#define MAX_KEYS 15
#define SKIP_KEYS_TIME 250
#define SKIP_KEYS 3
#define DIGITS_START_POS 5

extern word valid_chars[];
extern void set_value(byte* buffer, char* value);
extern void get_value(byte* buffer, char* value);

#define RES_NAME 5
#define RES_LATITUDE 6
#define RES_LONGITUDE 7

#define SEL_NAME 0
#define SEL_LAT 1
#define SEL_LON 2

// prototypes

void dispatch_edit_waypoint_screen(struct gesture_ *gesture);
void draw_edit_waypoint_screen();

#endif
