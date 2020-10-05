#include "gps_returner.h"
#include "edit_waypoint_screen.h"

word valid_chars[] = { ' ','+','-',':','.','0','1','2','3','4','5','6','7','8','9',0x90d0,0x91d0,0x92d0,0x93d0,0x94d0,0x95d0,0x81d0,0x96d0,0x97d0,0x98d0,0x99d0,0x9Ad0,0x9bd0,0x9cd0,0x9dd0,0x9ed0,0x9fd0,0xa0d0,0xa1d0,0xa2d0,0xa3d0,0xa4d0,0xa5d0,0xa6d0,0xa7d0,0xa8d0,0xa9d0,0xaad0,0xabd0,0xacd0,0xadd0,0xaed0,0xafd0,'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z' };

void set_value(byte* buffer, char* value)
{
    *buffer = 0;

    int i = 0;
    int offset = 1;
    while (true)
    {
        word w = 0;
        if (*(value + i) == 0x00)
            break;
        w = (byte)*(value + i) ;
        if ((byte) * (value + i) >= 0x80)
        {
            i++;
            w |= ((byte)*(value + i)) << 8;
        }

        int found = 0;
        for (int j = 0; j < sizeof(valid_chars)/2; j++)
        {
            if (valid_chars[j] == w)
            {
                *(buffer + offset++) = j;
                (*buffer)++;
                found = 1;
                break;
            }
        }
        // заменяем ненайденные символы пробелами
        if (!found)
        {
            *(buffer + offset++) = 0;
            (*buffer)++;
        }
        i++;

        if (*buffer == 15)
            break;
    }
}

void get_value(byte* buffer, char *value)
{
    _memclr(value, 32);

    for (int i = 1; i <= *buffer; i++)
    {
        _memcpy(value, &valid_chars[*(buffer+i)], 2);
        value++;
        // если символ двухбайтный, корректируем
        if (*value)
            value++;
    }
}

byte get_next_symbol()
{
    int skip = get_tick_count() - appdata->edit_tick < SKIP_KEYS_TIME ? SKIP_KEYS : 1;
    appdata->edit_tick = get_tick_count();

    if (appdata->edit_selected_field == SEL_NAME)
    {
        int len = sizeof(valid_chars) / 2;
        return (appdata->edit_name[appdata->edit_position+1] + skip) % len;
    }

    byte* p = appdata->edit_selected_field == SEL_LAT ? appdata->edit_lat : appdata->edit_lon;

    if (appdata->edit_position == 0)
        return p[1] == 1 ? 2 : 1; // + и -

    return ((p[appdata->edit_position+1] - DIGITS_START_POS + 1) % 10) + DIGITS_START_POS; // only digits
}

byte get_previous_symbol()
{
    int skip = get_tick_count() - appdata->edit_tick < SKIP_KEYS_TIME ? SKIP_KEYS : 1;
    appdata->edit_tick = get_tick_count();

    if (appdata->edit_selected_field == SEL_NAME)
    {
        int len = sizeof(valid_chars) / 2;
        return (appdata->edit_name[appdata->edit_position+1] + len - skip) % len;
    }

    byte* p = appdata->edit_selected_field == SEL_LAT ? appdata->edit_lat : appdata->edit_lon;

    if (appdata->edit_position == 0)
        return p[1] == 1 ? 2 : 1; // + и -

    return ((p[appdata->edit_position+1] + 10 - DIGITS_START_POS - 1) % 10) + DIGITS_START_POS; // only digits
}

byte get_next_right_position()
{
    if (appdata->edit_selected_field == SEL_NAME)
    {
        if (appdata->edit_position == 14)
            return 14;

        appdata->edit_position++;

        if (appdata->edit_position == appdata->edit_name[0])
        {
            appdata->edit_name[appdata->edit_position+1] = 0;
            appdata->edit_name[0]++;
        }
        return appdata->edit_position;
    }

    if (appdata->edit_position == 10)
        return 10;

    if (appdata->edit_position == 3)
        return 5;

    return appdata->edit_position + 1;
}

void trim_name()
{
    int len = appdata->edit_name[0];
    for (int i = len; i > 1; i--)
    {
        if (appdata->edit_name[i] == 0)
            appdata->edit_name[0]--;
        else
            break;
    }
}

byte get_next_left_position()
{
    if (appdata->edit_position == 0)
        return 0;

    if (appdata->edit_selected_field == SEL_NAME)
    {
        int res = appdata->edit_position - 1;
        trim_name();
        return res == 0 ? 0 : res - 1 >= appdata->edit_name[0] ? appdata->edit_name[0] - 1 : res;
    }

    if (appdata->edit_position == 5)
        return 3;

    return appdata->edit_position - 1;
}

float parse_coords(byte *value, int border)
{
    int f = 0;
    int d = 100000000;
    for (int i = 2; i < 12; i++)
    {
        if (i == 5)
            continue;
        f += d * (value[i] - DIGITS_START_POS);
        d = d / 10;
    }

    return (value[1] == 1? 1 : -1) * (f % (border * 1000000)) / 1000.0f / 1000.0f;
}

void dispatch_edit_waypoint_screen(struct gesture_* gesture)
{
    byte* p_edit_value;

    switch (appdata->edit_selected_field)
    {
    case SEL_NAME:
        p_edit_value = appdata->edit_name;
        break;
    case SEL_LAT:
        p_edit_value = appdata->edit_lat;
        break;
    case SEL_LON:
        p_edit_value = appdata->edit_lon;
        break;
    default:
        return;
    }

    switch (gesture->gesture)
    {
    case GESTURE_SWIPE_DOWN:
        p_edit_value[appdata->edit_position+1] = get_previous_symbol();
        break;
    case GESTURE_SWIPE_UP:
        p_edit_value[appdata->edit_position+1] = get_next_symbol();
        break;
    case GESTURE_SWIPE_LEFT:
        appdata->edit_position = get_next_left_position();
        break;
    case GESTURE_SWIPE_RIGHT:
        appdata->edit_position = get_next_right_position();
        break;
    case GESTURE_CLICK:
        if (gesture->touch_pos_y > 25 && gesture->touch_pos_y < 63 && appdata->edit_selected_field != SEL_NAME)
        {
            appdata->edit_position = 0;
            appdata->edit_selected_field = SEL_NAME;
        }
        else if (gesture->touch_pos_y > 63 && gesture->touch_pos_y < 101 && appdata->edit_selected_field != SEL_LAT)
        {
            appdata->edit_position = 0;
            appdata->edit_selected_field = SEL_LAT;
        }
        else if (gesture->touch_pos_y > 101 && gesture->touch_pos_y < 140 && appdata->edit_selected_field != SEL_LON)
        {
            appdata->edit_position = 0;
            appdata->edit_selected_field = SEL_LON;
        }
        trim_name();

        if (gesture->touch_pos_y > 140)
        {
            appdata->screen = SCREEN_LIST_WAYPOINTS;

            if (gesture->touch_pos_x > 88)
            {
                // new point?
                if (appdata->add_edit_index == -1)
                {
                    _memcpy(appdata->waypoints[appdata->waypoint_count].name, appdata->edit_name, 16);
                    appdata->waypoints[appdata->waypoint_count].location.latitude = parse_coords(appdata->edit_lat, 90);
                    appdata->waypoints[appdata->waypoint_count].location.longitude = parse_coords(appdata->edit_lon, 180);
                    appdata->selected_waypoint_index = appdata->waypoint_count;
                    appdata->waypoint_count++;
                    if (appdata->top_screen_waypoint_index + 2 < appdata->waypoint_count)
                    {
                        appdata->top_screen_waypoint_index = appdata->waypoint_count - 2;
                    }
                }
                else
                {
                    _memcpy(appdata->waypoints[appdata->add_edit_index].name, appdata->edit_name, 16);
                    appdata->waypoints[appdata->add_edit_index].location.latitude = parse_coords(appdata->edit_lat, 90);
                    appdata->waypoints[appdata->add_edit_index].location.longitude = parse_coords(appdata->edit_lon, 180);
                }

                write_settings();
            }
        }

        break;
    }

    draw_edit_waypoint_screen();
}

void draw_edit_waypoint_screen()
{
    char buffer[32];
    byte* p_edit_value;
    int y_offset = 0;

    set_fg_color(COLOR_WHITE);
    draw_filled_rect(0, 0, 175, 25);

    draw_horizontal_line(63, 0, 176);
    draw_horizontal_line(101, 0, 176);

    set_fg_color(COLOR_BLACK);
    set_bg_color(COLOR_WHITE);
    text_out_center(appdata->selected_waypoint_index == -1 ? "ДОБАВЛЕНИЕ" : "РЕДАКТИРОВАНИЕ", 88, 1 + (26 - get_text_height()) / 2);

    set_fg_color(COLOR_RED);
    draw_filled_rect(0, 140, 86, 175);

    set_fg_color(COLOR_GREEN);
    draw_filled_rect(89, 140, 175, 175);

    show_elf_res_by_id(ELF_INDEX_SELF, RES_DEL, 37, 151);
    show_elf_res_by_id(ELF_INDEX_SELF, RES_ACCEPT, 126, 151);

    int height = get_text_height();

    set_bg_color(COLOR_BLACK);
    switch (appdata->edit_selected_field)
    {
    case SEL_NAME:
        p_edit_value = appdata->edit_name;
        y_offset = 47;
        get_value(appdata->edit_lat, buffer);
        text_out_center(buffer, 88, 81 - height / 2);
        get_value(appdata->edit_lon, buffer);
        text_out_center(buffer, 88, 119 - height / 2);
        break;
    case SEL_LAT:
        p_edit_value = appdata->edit_lat;
        y_offset = 81;
        get_value(appdata->edit_name, buffer);
        text_out_center(buffer, 88, 47 - height / 2);
        get_value(appdata->edit_lon, buffer);
        text_out_center(buffer, 88, 119 - height / 2);
        break;
    case SEL_LON:
        p_edit_value = appdata->edit_lon;
        y_offset = 119;
        get_value(appdata->edit_name, buffer);
        text_out_center(buffer, 88, 47 - height / 2);
        get_value(appdata->edit_lat, buffer);
        text_out_center(buffer, 88, 81 - height / 2);
        break;
    default:
        return;
    }

    set_fg_color(COLOR_WHITE);

    int width_total = 16 + 8;
    int width_right = 0;
    int width_left = 0;

    for (int i = 1; i <= p_edit_value[0]; i++)
    {
        _memcpy(buffer, &valid_chars[p_edit_value[i]], 2);
        buffer[2] = 0;
        int width = text_width(buffer);
        width_total += width;
        if (i-1 < appdata->edit_position)
            width_left += width;
        if (i-1 > appdata->edit_position)
            width_right += width;
    }

    int start_x = 88 - width_total / 2;
    for (int i = 1; i <= p_edit_value[0]; i++)
    {
        _memcpy(buffer, &valid_chars[p_edit_value[i]], 2);
        buffer[2] = 0;
        int width = text_width(buffer);

        if (i-1 == appdata->edit_position)
        {
            draw_rect(start_x + 4, y_offset - height/ 2 - 5, start_x + 24, y_offset + height / 2 + 4);
            start_x += 16 - width / 2;
        }

        text_out(buffer, start_x, y_offset - height / 2);

        if (i-1 == appdata->edit_position)
        {
            start_x += -width / 2 + 16;
        }

        start_x += width;
    }
}
