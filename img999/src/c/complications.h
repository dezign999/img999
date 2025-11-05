#pragma once
#include <pebble.h>
#include "img999.h"

// Update complications text layers based on settings and buffers.
void updateComplications(const ClaySettings *settings,
                         TextLayer *day_layer,
                         TextLayer *date_layer,
                         const char *day_buffer,
                         const char *dateText,
                         const char *dayDate_buffer,
                         const char *month_buffer,
                         const char *short_date_buffer,
                         const char *steps_buffer);
