#include "complications.h"

void updateComplications(const ClaySettings *settings,
                         TextLayer *day_layer,
                         TextLayer *date_layer,
                         const char *day_buffer,
                         const char *dateText,
                         const char *dayDate_buffer,
                         const char *month_buffer,
                         const char *short_date_buffer,
                         const char *steps_buffer) {

    // Set fonts according to settings (14 or 24 bold)
    text_layer_set_font(day_layer, fonts_get_system_font((settings->comp1 == 51) ? FONT_KEY_GOTHIC_14_BOLD : FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_font(date_layer, fonts_get_system_font((settings->comp2 == 51) ? FONT_KEY_GOTHIC_14_BOLD : FONT_KEY_GOTHIC_24_BOLD));

    switch (settings->comp2) {
        case 49:
            text_layer_set_text(date_layer, day_buffer);
            break;
        case 50:
            text_layer_set_text(date_layer, dateText);
            break;
        case 51:
            text_layer_set_text(date_layer, dayDate_buffer);
            break;
        case 52:
            text_layer_set_text(date_layer, month_buffer);
            break;
        case 53:
            text_layer_set_text(date_layer, short_date_buffer);
            break;
        case 54:
            text_layer_set_text(date_layer, steps_buffer);
            break;
        default:
            layer_set_hidden(text_layer_get_layer(date_layer), true);
            break;
    }

    switch (settings->comp1) {
        case 49:
            text_layer_set_text(day_layer, day_buffer);
            break;
        case 50:
            text_layer_set_text(day_layer, dateText);
            break;
        case 51:
            text_layer_set_text(day_layer, dayDate_buffer);
            break;
        case 52:
            text_layer_set_text(day_layer, month_buffer);
            break;
        case 53:
            text_layer_set_text(day_layer, short_date_buffer);
            break;
        case 54:
            text_layer_set_text(day_layer, steps_buffer);
            break;
        default:
            layer_set_hidden(text_layer_get_layer(day_layer), true);
            break;
    }
}
