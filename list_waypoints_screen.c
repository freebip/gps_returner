#include"gps_returner.h"
#include "list_waypoints_screen.h"
#include "edit_waypoint_screen.h"

const char* format = "%+011.6f";

void dispatch_list_waypoints_screen(struct gesture_* gesture)
{
    char buffer[30];

    switch(gesture->gesture)
    {
    case GESTURE_SWIPE_LEFT:
        appdata->screen = SCREEN_MAIN;
        appdata->delete_click_count = 0;
        appdata->show_waypoint_time = 0;
        break;
    case GESTURE_SWIPE_RIGHT:
        if (appdata->selected_waypoint_index != -1)
        {
            appdata->add_edit_index = appdata->selected_waypoint_index;
            appdata->edit_selected_field = SEL_NAME;
            appdata->screen = SCREEN_EDIT_WAYPOINT;
            _memcpy(&appdata->edit_name, &appdata->waypoints[appdata->selected_waypoint_index].name, 16);

            _sprintf(buffer, format, appdata->waypoints[appdata->selected_waypoint_index].location.latitude);
            set_value(appdata->edit_lat, buffer);
            _sprintf(buffer, format, appdata->waypoints[appdata->selected_waypoint_index].location.longitude);
            set_value(appdata->edit_lon, buffer);

        }
        appdata->delete_click_count = 0;
        break;
    case GESTURE_CLICK:
        if (appdata->waypoint_count == 0)
        {
            if (gesture->touch_pos_y > 140)
            {
                appdata->screen = SCREEN_EDIT_WAYPOINT;
                appdata->edit_selected_field = SEL_NAME;
                appdata->add_edit_index = -1;

                set_value(appdata->edit_name, "ТОЧКА");
                
                if (appdata->location_data_ready)
                {
                    _sprintf(buffer, format, appdata->current_location.latitude);
                    set_value(appdata->edit_lat, buffer);
                    _sprintf(buffer, format, appdata->current_location.longitude);
                    set_value(appdata->edit_lon, buffer);
                }
                else
                {
                    _sprintf(buffer, format, 0.0f);
                    set_value(appdata->edit_lat, buffer);
                    _sprintf(buffer, format, 0.0f);
                    set_value(appdata->edit_lon, buffer);
                }

            }
            appdata->delete_click_count = 0;
        }
        else 
        {
            if (gesture->touch_pos_y > 140)
            {
                if (gesture->touch_pos_x > 88)
                {
                    appdata->screen = SCREEN_EDIT_WAYPOINT;
                    appdata->edit_selected_field = SEL_NAME;
                    appdata->add_edit_index = -1;

                    set_value(appdata->edit_name, "ТОЧКА");

                    if (appdata->location_data_ready)
                    {
                        _sprintf(buffer, format, appdata->current_location.latitude);
                        set_value(appdata->edit_lat, buffer);
                        _sprintf(buffer, format, appdata->current_location.longitude);
                        set_value(appdata->edit_lon, buffer);
                    }
                    else
                    {
                        _sprintf(buffer, format, 0.0f);
                        set_value(appdata->edit_lat, buffer);
                        _sprintf(buffer, format, 0.0f);
                        set_value(appdata->edit_lon, buffer);
                    }

                    appdata->delete_click_count = 0;
                }
                else 
                {
                    appdata->delete_click_count++;
                    if (appdata->delete_click_count == 2)
                    {
                        if (appdata->selected_waypoint_index < appdata->waypoint_count - 1)
                        {
                            for (int i = 0; i < (appdata->waypoint_count - appdata->selected_waypoint_index - 1); i++)
                                _memcpy(&appdata->waypoints[i+appdata->selected_waypoint_index], &appdata->waypoints[i+appdata->selected_waypoint_index + 1], sizeof(struct waypoint_t));
                        }
                        else
                            appdata->selected_waypoint_index = appdata->waypoint_count - 1;
                        appdata->waypoint_count--;

                        if (appdata->waypoint_count == 0)
                            appdata->selected_waypoint_index = -1;

                        if (appdata->top_screen_waypoint_index + 2 >= appdata->waypoint_count && appdata->top_screen_waypoint_index)
                            appdata->top_screen_waypoint_index--;

                        appdata->delete_click_count = 0;

                        write_settings();
                    }
                }
            }
            else
            {
                for (int i = 0;i < 2; i++)
                {
                    if (gesture->touch_pos_y > 28 + i * 56 && gesture->touch_pos_y < 82 + i * 56)
                    {
                        if (appdata->top_screen_waypoint_index + i < appdata->waypoint_count)
                        {
                            appdata->selected_waypoint_index = appdata->top_screen_waypoint_index + i;
                            write_settings();
                            break;
                        }
                    }
                }


                appdata->delete_click_count = 0;
            }

        }
        break;
    case GESTURE_SWIPE_DOWN:
        appdata->delete_click_count = 0;
        if (appdata->top_screen_waypoint_index > 0)
            appdata->top_screen_waypoint_index--;
        break;
    case GESTURE_SWIPE_UP:
        appdata->delete_click_count = 0;
        if (appdata->top_screen_waypoint_index + 2 < appdata->waypoint_count)
            appdata->top_screen_waypoint_index++;
        break;
    }

    draw_list_waypoints_screen();
}

void draw_list_waypoints_screen()
{
    char buffer[32];

    set_fg_color(COLOR_WHITE);
    draw_filled_rect(0, 0, 175, 25);

    if (appdata->waypoint_count > 0)
        draw_filled_rect(0, 82, 176, 83);

    set_fg_color(COLOR_BLACK);
    draw_filled_rect(0, 26, 175, 27);

    
    set_fg_color(COLOR_BLACK);
    set_bg_color(COLOR_WHITE);
    text_out_center("ПУТЕВЫЕ ТОЧКИ", 88,  1+(26 - get_text_height()) / 2);

    set_fg_color(COLOR_WHITE);
    set_bg_color(COLOR_BLACK);

    if (appdata->waypoint_count == 0)
    {

        text_out_center("Путевые точки", 88, 60);
        text_out_center("не заданы", 88, 90);

        set_fg_color(COLOR_GREEN);
        draw_filled_rect(0, 140, 175, 175);

        show_elf_res_by_id(ELF_INDEX_SELF, RES_ADD, 82, 151);
    }
    else 
    {

        set_fg_color(COLOR_RED);
        draw_filled_rect(0, 140, 86, 175);
        set_fg_color(COLOR_GREEN);
        draw_filled_rect(89, 140, 175, 175);

        if (appdata->selected_waypoint_index == -1)
            appdata->selected_waypoint_index = 0;
        else if (appdata->selected_waypoint_index > appdata->waypoint_count - 1)
            appdata->selected_waypoint_index = appdata->waypoint_count - 1;

        for (int i = 0; i < 2; i++)
        {
            set_fg_color(COLOR_WHITE);
            int index = appdata->top_screen_waypoint_index + i;

            if (appdata->selected_waypoint_index == index)
            {
                set_bg_color(COLOR_AQUA);
                draw_filled_rect_bg(0, 28 + i * 56, 176, 81 + i * 56);
            }
            else
            {
                set_bg_color(COLOR_BLACK);
            }

            get_value(appdata->waypoints[index].name, buffer);
            text_out(buffer, 6, 35 + i * 54);

            if (appdata->location_data_ready)
            {
                get_distance_string_to_waypoint(index, buffer);
                if (appdata->selected_waypoint_index == index)
                {
                    set_fg_color(COLOR_BLACK);
                }
                else
                {
                    set_fg_color(COLOR_AQUA);
                }
                text_out(buffer, 6, 60 + i * 54);
            }

            if (appdata->waypoint_count - 1 <= i + appdata->top_screen_waypoint_index)
                break;
        }

        if (appdata->waypoint_count > 2)
        {
            int size = 109 / (appdata->waypoint_count - 1) + 1;
            int position = 28 + appdata->top_screen_waypoint_index * 109 / (appdata->waypoint_count - 1);
            set_fg_color(COLOR_WHITE);
            draw_filled_rect(VIDEO_X - 4, 28, VIDEO_X, 137);
            set_fg_color(COLOR_BLUE);
            draw_filled_rect(VIDEO_X - 4, position, VIDEO_X, position+size);
        }



        if (!appdata->delete_click_count)
        {
            show_elf_res_by_id(ELF_INDEX_SELF, RES_DEL, 28, 151);
            show_elf_res_by_id(ELF_INDEX_SELF, RES_DEL, 45, 151);
        }
        else
        {
            show_elf_res_by_id(ELF_INDEX_SELF, RES_DEL, 37, 151);
        }
        show_elf_res_by_id(ELF_INDEX_SELF, RES_ADD, 126, 151);
    }
}