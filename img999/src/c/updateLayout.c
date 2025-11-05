#include "updateLayout.h"

void updateLayout(const ClaySettings *settings, LayoutValues *out) {
    if (!out || !settings) return;

    switch (settings->layout) {
        case 48:
            out->hour_x = 3;
            out->hour_y = 2;
            out->hour_width = 50;
            out->hour_height = 80;

            out->minute_x = 3;
            out->minute_y = 62;
            out->minute_width = 50;
            out->minute_height = 80;

            out->comp1_x = 1;
            out->comp1_y = (settings->comp1 == 51) ? 8 : -2;
            out->comp1_width = 48;
            out->comp1_height = 26;

            out->comp2_x = 1;
            out->comp2_y = (settings->comp2 == 51) ? 142 : 136;
            out->comp2_width = 48;
            out->comp2_height = 26;

            out->gradient_x = -94;
            out->gradient_y = 0;
            break;
        case 49:
            out->hour_x = 96;
            out->hour_y = 2;
            out->hour_width = 50;
            out->hour_height = 80;

            out->minute_x = 96;
            out->minute_y = 62;
            out->minute_width = 50;
            out->minute_height = 80;

            out->comp1_x = 94;
            out->comp1_y = (settings->comp1 == 51) ? 8 : -2;
            out->comp1_width = 48;
            out->comp1_height = 26;

            out->comp2_x = 94;
            out->comp2_y = (settings->comp2 == 51) ? 142 : 136;
            out->comp2_width = 48;
            out->comp2_height = 26;

            out->gradient_x = 93;
            out->gradient_y = 0;
            break;
        case 50:
            out->hour_x = 44;
            out->hour_y = -4;
            out->hour_width = 30;
            out->hour_height = 30;

            out->minute_x = 75;
            out->minute_y = -4;
            out->minute_width = 30;
            out->minute_height = 30;

            out->comp1_x = 3;
            out->comp1_y = (settings->comp1 == 51) ? 4 : -3;
            out->comp1_width = 40;
            out->comp1_height = 26;

            out->comp2_x = 100;
            out->comp2_y = (settings->comp2 == 51) ? 4 : -3;
            out->comp2_width = 40;
            out->comp2_height = 26;

            out->gradient_x = 0;
            out->gradient_y = -138;
            break;
        case 51:
            out->hour_x = 44;
            out->hour_y = 136;
            out->hour_width = 30;
            out->hour_height = 30;

            out->minute_x = 75;
            out->minute_y = 136;
            out->minute_width = 30;
            out->minute_height = 30;

            out->comp1_x = 3;
            out->comp1_y = (settings->comp1 == 51) ? 144 : 138;
            out->comp1_width = 40;
            out->comp1_height = 26;

            out->comp2_x = 100;
            out->comp2_y = (settings->comp2 == 51) ? 144 : 138;
            out->comp2_width = 40;
            out->comp2_height = 26;
            
            out->gradient_x = 0;
            out->gradient_y = 138;
            break;
        default:
            out->hour_x = 3;
            out->hour_y = 2;
            out->hour_width = 50;
            out->hour_height = 80;

            out->minute_x = 3;
            out->minute_y = 62;
            out->minute_width = 50;
            out->minute_height = 80;

            out->comp1_x = 1;
            out->comp1_y = (settings->comp1 == 51) ? 8 : -2;
            out->comp1_width = 48;
            out->comp1_height = 26;

            out->comp2_x = 1;
            out->comp2_y = (settings->comp2 == 51) ? 142 : 136;
            out->comp2_width = 48;
            out->comp2_height = 26;

            out->gradient_x = -94;
            out->gradient_y = 0;
        break;
    }
}
