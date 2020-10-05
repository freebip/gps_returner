#include <math.h>
#include "gps_returner.h"
#include "main_screen.h"
#include "edit_waypoint_screen.h"
#include "stack.h"

float PI = 3.14159265359f;

void dispatch_main_screen(struct gesture_* gest)
{
    if (gest->gesture == GESTURE_SWIPE_RIGHT)
        appdata->screen = SCREEN_LIST_WAYPOINTS;
    if (gest->gesture == GESTURE_CLICK)
        appdata->show_waypoint_time = 0;
}

float to_radian(float degree)
{
    return degree * PI / 180.0f;
}

int to_degree(float angle)
{
    return (int)(angle * 180.0f / PI);
}

int get_pixel(int x, int y)
{
    if (x >= VIDEO_X || y >= VIDEO_Y)
        return -1;

    void* screen = get_ptr_screen_memory();
    byte* address = (byte*)screen + 88 * y + (x >> 1);
    return 0xf & ((x % 2 != 0) ? *address >> 4 : *address);
}

void draw_pixel(int x, int y, int color)
{
    if (x >= VIDEO_X || y >= VIDEO_Y)
        return;

    void* screen = get_ptr_screen_memory();
    byte* address = (byte*)screen + 88 * y + (x >> 1);

    if (x % 2 != 0)
    {
        *address &= 0xf;
        *address |= (color & 0xf) << 4;
    }
    else
    {
        *address &= 0xf0;
        *address |= color & 0xf;
    }
}

void swap(int* a, int* b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void draw_line(int x0, int y0, int x1, int y1, int color)
{
    int steep = abssub(y1, y0) > abssub(x1, x0);
    if (steep)
    {
        swap(&x0, &y0);
        swap(&x1, &y1);
    }
    if (x0 > x1)
    {
        swap(&x0, &x1);
        swap(&y0, &y1);
    }
    int dx = x1 - x0;
    int dy = abssub(y1, y0);
    int error = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;
    for (int x = x0; x <= x1; x++)
    {
        draw_pixel(steep ? y : x, steep ? x : y, color);
        error -= dy;
        if (error < 0)
        {
            y += ystep;
            error += dx;
        }
    }
}

/*
void draw_circle(int x_center, int y_center, int radius, int color)
{
    int x = 0;
    int y = radius;
    int delta = 1 - 2 * radius;
    int error = 0;
    while (y > 0)
    {
        draw_pixel(x_center + x, y_center + y, color);
        draw_pixel(x_center + x, y_center - y, color);
        draw_pixel(x_center - x, y_center + y, color);
        draw_pixel(x_center - x, y_center - y, color);
        error = 2 * (delta + y) - 1;
        if (delta < 0 && error <= 0)
        {
            delta += 2 * ++x + 1;
            continue;
        }
        if (delta > 0 && error > 0)
        {
            delta -= 2 * --y + 1;
            continue;
        }
        delta += 2 * (++x - --y);
    }
}
*/

void fill(byte x, byte y, int color)
{
    struct stack_t stack;
    init(&stack);

    int old_color = get_pixel(x, y);
    if (color == old_color)
        return;

    struct point_t pt = { x, y };
    push(&stack, pt);

    while (stack.head)
    {
        pt = pop(&stack);
        word y1 = pt.y;
        while (y1 >= 1 && get_pixel(pt.x, y1) == old_color)
            y1--;
        y1++;
        int span_left = 0;
        int span_right = 0;

        while (y1 < VIDEO_Y && get_pixel(pt.x, y1) == old_color)
        {
            draw_pixel(pt.x, y1, color);
            if (!span_left && pt.x > 0 && get_pixel(pt.x - 1, y1) == old_color)
            {
                struct point_t new_pt = { pt.x - 1, y1 };
                push(&stack, new_pt);
                span_left = 1;
            }
            else if (span_left && pt.x > 0 && get_pixel(pt.x - 1, y1) != old_color)
            {
                span_left = 0;
            }

            if (!span_right && pt.x < VIDEO_X && get_pixel(pt.x + 1, y1) == old_color)
            {
                struct point_t new_pt = { pt.x + 1, y1 };
                push(&stack, new_pt);
                span_right = 1;
            }
            else if (span_right && pt.x < VIDEO_X && get_pixel(pt.x + 1, y1) != old_color)
            {
                span_right = 0;
            }
            y1++;
        }
    }
}

struct point_t rotate(float angle, struct point_t pt)
{
    struct point_t res;
    res.x = pt.x * cosf(angle) - pt.y * sinf(angle);
    res.y = pt.y * cosf(angle) + pt.x * sinf(angle);
    return res;
}

void draw_arrow(int x, int y, int color, int direction_angle)
{
    const int ARROW_SIZE = 10;
    struct point_t points[] = { {-3 * ARROW_SIZE, 2 * ARROW_SIZE}, {0, -4 * ARROW_SIZE}, {3 * ARROW_SIZE, 2 * ARROW_SIZE}, {0, 1 * ARROW_SIZE} };
    float angle = to_radian(direction_angle);

    for (int i = 0; i < 4; i++)
    {
        struct point_t pt0 = rotate(angle, points[i]);
        struct point_t pt1 = rotate(angle, points[(i + 1) % 4]);
        draw_line(x + pt0.x, y + pt0.y, x + pt1.x, y + pt1.y, color);
    }

    struct point_t pt_pixel = { 0, ARROW_SIZE * 6 };
    for (int i = 0; i < 12; i++)
    {
        struct point_t pt = rotate(PI / 6 * i , pt_pixel );

        if (i%3)
            draw_pixel(x + pt.x, y + pt.y, COLOR_SH_WHITE);
        else
        {
            draw_line(x + pt.x - 1, y + pt.y, x + pt.x + 1, y + pt.y, COLOR_SH_WHITE);
            draw_line(x + pt.x, y + pt.y - 1, x + pt.x, y + pt.y + 1, COLOR_SH_WHITE);
        }
    }

}

void get_waypoint_name(int index, char* buffer)
{
    if (index == -1)
    {
        *buffer = 0;
        return;
    }

    int offset = 0;
    for (int i = 0; i < 15; i++)
    {
        if (appdata->waypoints[index].name[i])
            break;
        _memcpy(buffer+offset, &appdata->waypoints[index].name[i], 2);
        offset += *(buffer + offset + 1) ? 2 : 1;
    }
    *(buffer + offset) = 0;
}

float get_distance(struct location_t a, struct location_t b)
{
    float a_lat = to_radian(a.latitude);
    float a_lon = to_radian(a.longitude);
    float b_lat = to_radian(b.latitude);
    float b_lon = to_radian(b.longitude);

    const int RAD = 6372795;

    float cl1 = cosf(a_lat);
    float cl2 = cosf(b_lat);
    float sl1 = sinf(a_lat);
    float sl2 = sinf(b_lat);
    float delta = b_lon - a_lon;
    float cdelta = cosf(delta);
    float sdelta = sinf(delta);
    float y = sqrtf(powf(cl2 * sdelta, 2) + powf(cl1 * sl2 - sl1 * cl2 * cdelta, 2));
    float x = sl1 * sl2 + cl1 * cl2 * cdelta;
    float ad = atan2f(y, x);
    return ad * RAD;
}

void get_distance_string_to_waypoint(int index, char* buffer)
{
    float distance = index == -1 || !appdata->location_data_ready  ? 0 : get_distance(appdata->waypoints[index].location, appdata->current_location);
    if (distance > 1e6)
        _sprintf(buffer, "%d км", (int)(distance / 1000));
    else if (distance > 1000)
        _sprintf(buffer, "%.2f км", distance / 1000.0);
    else
        _sprintf(buffer, "%d м", (int)distance);
}

void show_time(int x, int y)
{
    struct datetime_ datetime;
    struct res_params_ res_params;

    get_current_date_time(&datetime);
    int nres = datetime.hour / 10;
    get_res_params(ELF_INDEX_SELF, nres, &res_params);
    show_elf_res_by_id(ELF_INDEX_SELF, nres, x, y);
    x += res_params.width + 2;
    nres = datetime.hour % 10;
    get_res_params(ELF_INDEX_SELF, nres, &res_params);
    show_elf_res_by_id(ELF_INDEX_SELF, nres, x, y);
    x += res_params.width + 2;
    show_elf_res_by_id(ELF_INDEX_SELF, 10, x, y+2);
    x += 4;
    nres = datetime.min / 10;
    get_res_params(ELF_INDEX_SELF, nres, &res_params);
    show_elf_res_by_id(ELF_INDEX_SELF, nres, x, y);
    x += res_params.width + 2;
    nres = datetime.min % 10;
    show_elf_res_by_id(ELF_INDEX_SELF, nres, x, y);
}

int get_decimal_max(int value)
{
    if (value == 0)
        return 1;

    int max = 100000;

    while (max)
    {
        if (value / max)
            break;
        max = max / 10;
    }
    return max;
}

void show_speed(int x, int y)
{
    struct res_params_ res_params;

    // kmh
    show_elf_res_by_id(ELF_INDEX_SELF, 27, x, y);
    // speed int part
    int value = (int)appdata->speed;
    int max = get_decimal_max(value);
    while (max)
    {
        int d = value / max;
        get_res_params(ELF_INDEX_SELF, 11 + d, &res_params);
        show_elf_res_by_id(ELF_INDEX_SELF, 11 + d, x, y + 12);
        x += res_params.width + 3;
        value = value % max;
        max = max / 10;
    }
    show_elf_res_by_id(ELF_INDEX_SELF, 21, x, y + 12 + 18);
    x += 5;
    int d = ((int)(appdata->speed * 10)) % 10;
    show_elf_res_by_id(ELF_INDEX_SELF, 11 + d, x, y + 12);
}

void show_distance(int x, int y)
{
    struct res_params_ res_params;

    float distance = get_distance(appdata->waypoints[appdata->selected_waypoint_index].location, appdata->current_location);
    int nres = distance > 1e3 ? 26 : 28;
    get_res_params(ELF_INDEX_SELF, nres, &res_params);
    show_elf_res_by_id(ELF_INDEX_SELF, nres, x - res_params.width, y);

    if (distance > 1e3 && distance < 1e6)
    {
        int d = ((int)((distance + 50) / 100)) % 10;
        get_res_params(ELF_INDEX_SELF, 11+d, &res_params);
        x -= res_params.width;
        show_elf_res_by_id(ELF_INDEX_SELF, 11 + d, x, y + 12);
        x -= 5;
        show_elf_res_by_id(ELF_INDEX_SELF, 21, x, y + 12 + 18);
        x -= 2;
    }

    int value = (int)(distance >= 1e3 ? distance / 1000 : distance);

    do
    { 
        int d = value % 10;
        get_res_params(ELF_INDEX_SELF, 11 + d, &res_params);
        x -= res_params.width;
        show_elf_res_by_id(ELF_INDEX_SELF, 11 + d, x, y + 12);
        x -= 3;
        value = value / 10;
    } while (value);

}

void show_battery(int x, int y)
{
    struct res_params_ res_params;
    get_res_params(ELF_INDEX_SELF, 29, &res_params);
    x -= res_params.width;
    show_elf_res_by_id(ELF_INDEX_SELF, 29, x, y);

    if (get_fw_version() != 11536)
    {
        set_bg_color(COLOR_YELLOW);
        draw_filled_rect_bg(x + 2, y + 2, x + 21, y + 10);
    }
    else 
    {
#ifdef BIPEMULATOR
        word battery_percentage = 80;
#else
        word battery_percentage = *((word*)(0x20000334));
#endif

        int r_count = battery_percentage / 20;
        r_count = r_count > 4 ? 4 : r_count < 1 ? 1 : r_count;
        set_bg_color(battery_percentage < 20 ? COLOR_RED : COLOR_GREEN);
        for (int i = 0; i < r_count; i++)
        {
            draw_filled_rect_bg(x + 2 + i * 5, y + 2, x + 5 + i * 5, y + 9);
        }

        x -= 3;

        do
        {
            int d = battery_percentage % 10;
            get_res_params(ELF_INDEX_SELF, d, &res_params);
            x -= res_params.width;
            show_elf_res_by_id(ELF_INDEX_SELF, d, x, y + 1);
            x -= 2;
            battery_percentage = battery_percentage / 10;
        } while (battery_percentage);
    }
}

int normalize_degree(int angle)
{
    while(angle < 0)
        angle += 360;

    angle = angle % 360;

    return angle;
}


void draw_main_screen()
{
    char buffer[32];
    int point_dir = 0, delta_dir=0;

    // time
    show_time(5, 9);

    // battery
    show_battery(170, 8);

    if (appdata->selected_waypoint_index != -1)
    {
        point_dir = normalize_degree(270 - get_direction(appdata->waypoints[appdata->selected_waypoint_index].location, appdata->current_location));
        delta_dir = normalize_degree(point_dir - appdata->direction_angle);

    }

#ifndef BIPEMULATOR
    int color = appdata->location_data_ready ? COLOR_SH_WHITE : COLOR_SH_RED;
    if (appdata->selected_waypoint_index == -1)
    {
        // если точка не выбрана, то показываем на север
        draw_arrow(88, 88, color, normalize_degree(-appdata->direction_angle));
    }
    else
    {
        // указываем на точку возврата
        draw_arrow(88, 88, color, delta_dir);
    }
    fill(88, 80, color);

#endif

#ifdef DEBUG
    
    set_bg_color(COLOR_BLACK);
    set_fg_color(COLOR_YELLOW);

    _sprintf(buffer, "move dir: %d", appdata->direction_angle);
    text_out(buffer, 5, 28);

    _sprintf(buffer, "point dir: %d", point_dir);
    text_out(buffer, 5, 48);

    _sprintf(buffer, "res dir: %d", delta_dir);
    text_out(buffer, 5, 68);

    set_fg_color(COLOR_WHITE);
#endif

    // speed
    if (appdata->location_data_ready)
        show_speed(5, 135);

    // distance
    if (appdata->selected_waypoint_index != -1 && appdata->location_data_ready)
        show_distance(170, 135);

    if (appdata->selected_waypoint_index != -1 && appdata->show_waypoint_time < 2)
    {
        set_fg_color(COLOR_GREEN);
        draw_filled_rect(0, 88 - 15, 175, 88 + 15);
        set_fg_color(COLOR_WHITE);
        draw_rect(0, 88 - 15, 175, 88 + 15);
        get_value(appdata->waypoints[appdata->selected_waypoint_index].name, buffer);
        text_out_center(buffer, 88, 88 - get_text_height() / 2);
        appdata->show_waypoint_time++;
    }
}

