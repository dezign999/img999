#pragma once
#include <pebble.h>
#include "img999.h"

typedef struct {
    int hour_x, hour_y, hour_width, hour_height;
    int minute_x, minute_y, minute_width, minute_height;
    int comp1_x, comp1_y, comp1_width, comp1_height;
    int comp2_x, comp2_y, comp2_width, comp2_height;
    int gradient_x, gradient_y;
} LayoutValues;

// Populate `out` based on the provided settings.
void updateLayout(const ClaySettings *settings, LayoutValues *out);
