#include <math.h>
#include "libbip.h"
#include "gps_returner.h"
#include "main_screen.h"
#include "list_waypoints_screen.h"
#include "edit_waypoint_screen.h"

struct regmenu_ menu_screen = { 55, 1, 0, dispatch_screen, keypress_screen, screen_job, 0, show_screen, 0, 0 };

struct appdata_t** appdata_p;
struct appdata_t* appdata;

int main(int p, char** a)
{
    show_screen((void*)p);
}

void read_settings()
{
    ElfReadSettings(ELF_INDEX_SELF, &appdata->waypoint_count, 0, 4);
    if (appdata->waypoint_count > 10)
        appdata->waypoint_count = 0;
    ElfReadSettings(ELF_INDEX_SELF, &appdata->selected_waypoint_index, 4, 4);
    if (!appdata->waypoint_count)
        appdata->selected_waypoint_index = -1;
    else if (appdata->waypoint_count < appdata->selected_waypoint_index)
        appdata->selected_waypoint_index = 0;
    if (appdata->waypoint_count > 0)
        ElfReadSettings(ELF_INDEX_SELF, &appdata->waypoints, 8, appdata->waypoint_count * sizeof(struct waypoint_t));
}

void write_settings()
{
    ElfWriteSettings(ELF_INDEX_SELF, &appdata->waypoint_count, 0, 4);
    ElfWriteSettings(ELF_INDEX_SELF, &appdata->selected_waypoint_index, 4, 4);
    ElfWriteSettings(ELF_INDEX_SELF, &appdata->waypoints, 8, appdata->waypoint_count * sizeof(struct waypoint_t));
}

void show_screen(void* p)
{
    appdata_p = (struct appdata_t**)get_ptr_temp_buf_2();

    if ((p == *appdata_p) && get_var_menu_overlay()) {
        appdata = *appdata_p;
        *appdata_p = (struct appdata_t*)NULL;
        reg_menu(&menu_screen, 0);
        *appdata_p = appdata;
    }
    else {
        reg_menu(&menu_screen, 0);
        *appdata_p = (struct appdata_t*)pvPortMalloc(sizeof(struct appdata_t));
        appdata = *appdata_p;
        _memclr(appdata, sizeof(struct appdata_t));
        appdata->proc = (Elf_proc_*)p;

        read_settings();
    }

    if (p && appdata->proc->elf_finish)
        appdata->ret_f = appdata->proc->elf_finish;
    else
        appdata->ret_f = show_watchface;

    appdata->screen = SCREEN_MAIN;
    appdata->top_screen_waypoint_index = 0;

    switch_gps_pressure_sensors(1);

    draw_screen();

    set_display_state_value(8, 1);
    set_display_state_value(2, 1);

    set_update_period(1, 1000); 
}

void keypress_screen()
{
    // gps off
    switch_gps_pressure_sensors(0);
    show_menu_animate(appdata->ret_f, (unsigned int)show_screen, ANIMATE_RIGHT);
};

int dispatch_screen(void* p)
{
    struct gesture_* gest = (struct gesture_*)p;

    switch (appdata->screen)
    {
    case SCREEN_MAIN:
        dispatch_main_screen(gest);
        break;
    case SCREEN_LIST_WAYPOINTS:
        dispatch_list_waypoints_screen(gest);
        break;
    case SCREEN_EDIT_WAYPOINT:
        dispatch_edit_waypoint_screen(gest);
        break;
    default:
        return 0;
    }

    draw_screen();
    return 0;
}

float normalize_radians(float angle)
{
    float twopi = 2 * PI;

    while (angle < 0)
        angle += twopi;

    while(angle > twopi)
        angle -= twopi;

    return angle;
}

int get_direction(struct location_t from, struct location_t to)
{
    float to_lat = to_radian(to.latitude);
    float to_lon = to_radian(to.longitude);
    float from_lat = to_radian(from.latitude);
    float from_lon = to_radian(from.longitude);
    float angle = atan2f(to_lat - from_lat, to_lon - from_lon);
    return to_degree(normalize_radians(angle));
}

void screen_job()
{
    struct datetime_ dt;
    navi_struct_ navi_data;
    get_navi_data(&navi_data);

    if (IS_NAVI_GPS_READY(navi_data.ready))
    {
        get_current_date_time(&dt);

        appdata->current_location.latitude = navi_data.latitude / 3000000.0 * (navi_data.ns ? 1 : -1);
        appdata->current_location.longitude = navi_data.longitude / 3000000.0 * (navi_data.ew == 2 ? -1 : 1);

        if (appdata->location_data_ready)
        {
            int ticks = get_tick_count();
            int delta = ticks - appdata->ticks;
            if (delta > DELTA_TICKS_FOR_SPEED)
            {
                appdata->speed = get_distance(appdata->current_location, appdata->location_for_speed) * 3.6f / delta * 510.0f;
                appdata->location_for_speed = appdata->current_location;
                appdata->ticks = ticks;
            }

            appdata->distance += get_distance(appdata->location_for_direction, appdata->current_location);
            if (appdata->distance > DISTANCE_FOR_DIRECTION)
            {
                appdata->direction_angle = normalize_degree(90 - get_direction(appdata->location_for_direction, appdata->current_location));
                appdata->location_for_direction = appdata->current_location;
                appdata->distance = 0;
            }
        }
        else
        {
            appdata->direction_angle = 0;
            appdata->location_for_direction = appdata->current_location;
            appdata->location_for_speed = appdata->current_location;
            appdata->distance = 0;
            appdata->ticks = get_tick_count();
        }
        appdata->location_data_ready = 1;
    }
    else
    {
        appdata->location_data_ready = 0;
    }

    draw_screen();
    set_update_period(1, 1000);
}

void draw_screen()
{
    set_graph_callback_to_ram_1();
    set_bg_color(COLOR_BLACK);
    fill_screen_bg();
    load_font();

    switch (appdata->screen)
    {
    case SCREEN_MAIN:
        draw_main_screen();
        break;
    case SCREEN_LIST_WAYPOINTS:
        draw_list_waypoints_screen();
        break;
    case SCREEN_EDIT_WAYPOINT:
        draw_edit_waypoint_screen();
        break;
    }


    repaint_screen_lines(0, 176);
}
