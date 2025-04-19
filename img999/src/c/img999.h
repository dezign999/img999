#pragma once
#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {

int chunk;
int background;
int hourlyvibe;
int index;
char message;
int size;
char url;
GColor hourColor;
GColor minuteColor;
char gradient;
GColor gradColor;
char comp1;
char comp2;
char layout;

} __attribute__((__packed__)) ClaySettings;

static void updateComplications();
void tick_handler(struct tm *tick_time, TimeUnits units_changed);