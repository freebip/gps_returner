#ifndef __MAIN_SCREEN_H__
#define __MAIN_SCREEN_H__

#include "libbip.h"

// prototypes

void draw_main_screen();
void dispatch_main_screen(struct gesture_* gest);
float to_radian(float degree);
int to_degree(float radians);
int normalize_degree(int degree);

#endif
