#ifndef __GPS_RETURNER__
#define __GPS_RETURNER__

//#define DEBUG

#include "libbip.h"

typedef unsigned short word;
typedef unsigned char byte;

struct location_t
{
    float latitude;
    float longitude;
};

struct waypoint_t
{
    // 0: count, 1..15: name
    byte name[16];
    struct location_t location;
};

struct appdata_t
{
    Elf_proc_* proc;
    void* ret_f;
    int screen;
    int selected_waypoint_index;
    int top_screen_waypoint_index;
    int location_data_ready;
    int waypoint_count;
    struct location_t current_location;
    struct location_t location_for_direction;
    struct location_t location_for_speed;
    struct waypoint_t waypoints[10];
    float speed;
    float distance;
    // in degree
    int direction_angle;
    int delete_click_count;
    int add_edit_index;
    int ticks;

    byte edit_name[16];
    byte edit_lat[16];
    byte edit_lon[16];
    int edit_position;
    int edit_key_count;
    int edit_tick;
    int edit_selected_field;

    int show_waypoint_time;

};

extern struct appdata_t** appdata_p;
extern struct appdata_t* appdata;

#define DISTANCE_FOR_DIRECTION 25
#define DELTA_TICKS_FOR_SPEED (510 * 5)

// res

#define RES_ADD 31
#define RES_DEL 32
#define RES_ACCEPT 30

// screen's

#define SCREEN_MAIN 0
#define SCREEN_LIST_WAYPOINTS 1
#define SCREEN_EDIT_WAYPOINT 2

extern float PI;

// prototypes

void show_screen(void* return_screen);
void keypress_screen();
int dispatch_screen(void* p);
void draw_screen();
void screen_job();
void read_settings();
void write_settings();

extern void get_waypoint_name(int index, char* buffer);
extern float get_distance(struct location_t a, struct location_t b);
extern void get_distance_string_to_waypoint(int index, char* buffer);
float normalize_radians(float angle);
int get_direction(struct location_t from, struct location_t to);

#endif
