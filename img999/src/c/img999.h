#pragma once
#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {

int chunk;
int background;
int hourlyVibe;
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
int startTime;
int endTime;
bool vibeStrong;

} __attribute__((__packed__)) ClaySettings;

void tick_handler(struct tm *tick_time, TimeUnits units_changed);
// updateComplications is internal to img999.c and intentionally not exposed here.
void tick_handler(struct tm *tick_time, TimeUnits units_changed);