#include <pebble.h>
#include <ctype.h>
#include "img999.h"
#include "health.h"
#include "updateLayout.h"
#include "complications.h"
#ifdef PBL_COLOR
	#include "gbitmap_color_palette_manipulator.h"
#endif

// Made possible with code from Gregoire Sage's Image Viewer. All image loading credit goes to him!
// https://github.com/gregoiresage/pebble-image-viewer

static Window 			*window;
static GBitmap      	*image = NULL;
static GBitmap      	*gradient = NULL;
static BitmapLayer  	*image_layer;
static BitmapLayer 		*gradient_layer;
GFont					time_font;
static TextLayer    	*info_text_layer;
TextLayer 				*hour_text_layer;
TextLayer 				*minute_text_layer;
TextLayer 				*date_text_layer;
TextLayer 				*day_text_layer;
static uint8_t      	*data_image = NULL;
static uint32_t     	data_size;
AppTimer 				*vibration_timer;
int 					loopCounter = 0;
int 					loopCount = 0;
int 					quarterLoop;
static char 			dateText[6];
static char 			day_buffer[4];
static char 			dayDate_buffer[8];
static char 			month_buffer[4];
static char 			steps_buffer[8];
static char 			short_date_buffer[8];
static bool 			isObstructed = false;
bool 					vibration_timer_active = false;
static const uint8_t 	minute_start = 62;
static const uint8_t 	minute_end = 6;
static const uint8_t 	hour_start = 96;
static const uint8_t 	hour_end = 50;
static const uint8_t 	minute_start_L = 62;
static const uint8_t 	minute_end_L = 6;
static const uint8_t 	hour_start_L = 3;
static const uint8_t 	hour_end_L = 50;
static const uint8_t 	horiz_time_end = 40;
static ClaySettings 	settings;
static int 				hour_x, hour_y, hour_width, hour_height, minute_x,  minute_y, minute_width, minute_height,
						comp1_x, comp1_y, comp1_width, comp1_height, comp2_x, comp2_y, comp2_width, comp2_height,
						gradient_x, gradient_y;

#define CHUNK_SIZE 6000

#if PBL_RECT
#define TIME_X 144
#define TIME_Y 64
#define INFO_X 144
#define SLIDE_X 144
#define SLIDE_Y 168
#define INFO_Y 58
#else
#define TIME_Y 70
#define TIME_X 180
#define SLIDE_X 180
#define SLIDE_Y 180
#define INFO_X 180
#define INFO_Y 64
#endif

static const uint32_t vibe_durations_strong[] = {100};
static const uint32_t vibe_durations_weak[] = {50};
static int vibration_delay = 600;

const VibePattern vibe_pattern_strong = {
    .durations = (uint32_t *)vibe_durations_strong,
    .num_segments = 1
};

const VibePattern vibe_pattern_weak = {
    .durations = (uint32_t *)vibe_durations_weak,
    .num_segments = 1
};

VibePattern get_vibe_pattern() {
    return settings.vibeStrong ? vibe_pattern_weak : vibe_pattern_strong;
}

// Populate layout values from settings and copy into local variables used by this file.
static void apply_layout_from_settings() {
	LayoutValues lv;
	updateLayout(&settings, &lv);

	hour_x = lv.hour_x;
	hour_y = lv.hour_y;
	hour_width = lv.hour_width;
	hour_height = lv.hour_height;

	minute_x = lv.minute_x;
	minute_y = lv.minute_y;
	minute_width = lv.minute_width;
	minute_height = lv.minute_height;

	comp1_x = lv.comp1_x;
	comp1_y = lv.comp1_y;
	comp1_width = lv.comp1_width;
	comp1_height = lv.comp1_height;

	comp2_x = lv.comp2_x;
	comp2_y = lv.comp2_y;
	comp2_width = lv.comp2_width;
	comp2_height = lv.comp2_height;

	gradient_x = lv.gradient_x;
	gradient_y = lv.gradient_y;
}

static void gbitmap_destroy_safe(GBitmap **bitmap) {
    if (*bitmap) {
        gbitmap_destroy(*bitmap);
        *bitmap = NULL;
    }
}

static void updateGradient() {

	gbitmap_destroy_safe(&gradient);

	if (settings.gradient == 49) {
		bitmap_layer_set_background_color(gradient_layer, settings.gradColor);
		#ifdef PBL_BW
			bitmap_layer_set_bitmap(gradient_layer, gradient);
			bitmap_layer_set_compositing_mode(gradient_layer, GCompOpAssign);
		#endif
	} else {
		bitmap_layer_set_background_color(gradient_layer, GColorClear);
	}	

	switch (settings.gradient) {
		case 50:
			#ifdef PBL_BW
				if (settings.layout == 50 || settings.layout == 51) {
					gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_2);
				} else {
					gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_0);
				}
			#endif

			#ifdef PBL_COLOR
				gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_0);
			#endif

			bitmap_layer_set_bitmap(gradient_layer, gradient);
			bitmap_layer_set_compositing_mode(gradient_layer, GCompOpSet);
			break;
		case 51:
			#ifdef PBL_BW
			if (settings.layout == 50 || settings.layout == 51) {
				gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_3);
			} else {
				gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_1);
			}
			#endif

			#ifdef PBL_COLOR
				gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_1);
			#endif

			bitmap_layer_set_bitmap(gradient_layer, gradient);
			bitmap_layer_set_compositing_mode(gradient_layer, GCompOpSet);
			break;
		default:
			if (!isObstructed)
				layer_set_hidden(bitmap_layer_get_layer(gradient_layer), (settings.gradient == 48) ? true : false);
			break;
	}

	#ifdef PBL_COLOR
		if (settings.gradient != 48 && settings.gradient != 49) {
				gbitmap_fill_all_except(GColorClear, settings.gradColor, true, gradient, gradient_layer);
		}
	#endif

}

// updateComplications() moved to its own module.

static void updateLayers() {
	apply_layout_from_settings();

	if (settings.layout == 48 || settings.layout == 49) {
		if (time_font)
			fonts_unload_custom_font(time_font);
		time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_80));
		text_layer_set_text_alignment(hour_text_layer, GTextAlignmentLeft);
		text_layer_set_text_alignment(minute_text_layer, GTextAlignmentLeft);
		text_layer_set_text_alignment(day_text_layer, GTextAlignmentCenter);
		text_layer_set_text_alignment(date_text_layer, GTextAlignmentCenter);
	} else {
		if (time_font)
			fonts_unload_custom_font(time_font);
		time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_29));
		text_layer_set_text_alignment(hour_text_layer, GTextAlignmentRight);
		text_layer_set_text_alignment(minute_text_layer, GTextAlignmentLeft);
		text_layer_set_text_alignment(day_text_layer, GTextAlignmentLeft);
		text_layer_set_text_alignment(date_text_layer, GTextAlignmentRight);
	}

	text_layer_set_font(hour_text_layer, time_font);
	text_layer_set_font(minute_text_layer, time_font);

	text_layer_set_text_color(hour_text_layer, settings.hourColor);
	text_layer_set_text_color(minute_text_layer, settings.minuteColor);
	text_layer_set_text_color(day_text_layer, settings.hourColor);
	text_layer_set_text_color(date_text_layer, settings.minuteColor);

	if (isObstructed) {
		layer_set_hidden(text_layer_get_layer(date_text_layer), true);
		layer_set_hidden(text_layer_get_layer(day_text_layer),  true);
		layer_set_frame(bitmap_layer_get_layer(gradient_layer), GRect(0, 0, 144, 168));
		
		
		if (settings.gradient == 48) {

			gbitmap_destroy_safe(&gradient);

			#ifdef PBL_PLATFORM_BASALT
				gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_0);
			#endif

			bitmap_layer_set_bitmap(gradient_layer, gradient);
			bitmap_layer_set_compositing_mode(gradient_layer, GCompOpSet);
			text_layer_set_text_color(hour_text_layer, GColorWhite);
			text_layer_set_text_color(minute_text_layer, GColorWhite);
			layer_set_hidden(bitmap_layer_get_layer(gradient_layer), false);
		}

		if (settings.gradient == 49) {
			window_set_background_color(window, settings.gradColor);
			layer_set_hidden(bitmap_layer_get_layer(gradient_layer), true);
			layer_set_hidden(bitmap_layer_get_layer(image_layer), true);
		}

		if(settings.layout == 48) {
			GRect hour = layer_get_frame(text_layer_get_layer(hour_text_layer));
			hour.origin.x = hour_start_L;
			hour.origin.y = 6;
			layer_set_frame(text_layer_get_layer(hour_text_layer), hour);

			GRect minute = layer_get_frame(text_layer_get_layer(minute_text_layer));
			minute.origin.x = hour_end_L;
			minute.origin.y = minute_end_L;
			layer_set_frame(text_layer_get_layer(minute_text_layer), minute);	
		} else if (settings.layout == 49) {
			GRect hour = layer_get_frame(text_layer_get_layer(hour_text_layer));
			hour.origin.x = hour_end;
			hour.origin.y = 6;
			layer_set_frame(text_layer_get_layer(hour_text_layer), hour);

			GRect minute = layer_get_frame(text_layer_get_layer(minute_text_layer));
			minute.origin.x = minute_x;
			minute.origin.y = minute_end;
			layer_set_frame(text_layer_get_layer(minute_text_layer), minute);
		} else if (settings.layout == 50 || settings.layout == 51) {
			GRect hour = layer_get_frame(text_layer_get_layer(hour_text_layer));
			hour.origin.y = horiz_time_end;
			layer_set_frame(text_layer_get_layer(hour_text_layer), hour);

			GRect minute = layer_get_frame(text_layer_get_layer(minute_text_layer));
			minute.origin.y = horiz_time_end;
			layer_set_frame(text_layer_get_layer(minute_text_layer), minute);
		}

	} else {
		
		layer_set_hidden(text_layer_get_layer(date_text_layer), (settings.comp2 == 48) ? true : false);
		layer_set_hidden(text_layer_get_layer(day_text_layer), (settings.comp1 == 48) ? true : false);

		window_set_background_color(window, GColorBlack);
		layer_set_hidden(bitmap_layer_get_layer(image_layer), false);
		
		#ifdef PBL_BW
			switch (settings.layout) {
				case 48:
					bitmap_layer_set_alignment(gradient_layer, GAlignLeft);
				break;
				case 49:
					bitmap_layer_set_alignment(gradient_layer, GAlignRight);
				break;
				case 50:
					bitmap_layer_set_alignment(gradient_layer, GAlignTop);
				break;
				case 51:
					bitmap_layer_set_alignment(gradient_layer, GAlignBottom);
				break;
				default:
					bitmap_layer_set_alignment(gradient_layer, GAlignLeft);
				break;
			}
		#endif

		layer_set_frame(text_layer_get_layer(hour_text_layer), GRect(hour_x, hour_y, hour_width, hour_height));
		layer_set_frame(text_layer_get_layer(minute_text_layer), GRect(minute_x, minute_y, minute_width, minute_height));
			
		layer_set_frame(text_layer_get_layer(date_text_layer), GRect(comp2_x, comp2_y, comp2_width, comp2_height));
		layer_set_frame(text_layer_get_layer(day_text_layer), GRect(comp1_x, comp1_y, comp1_width, comp1_height));
		
		#ifdef PBL_BW
		if (settings.gradient == 49) {
			layer_set_frame(bitmap_layer_get_layer(gradient_layer), GRect(gradient_x, gradient_y, 144, 168));
			bitmap_layer_set_compositing_mode(gradient_layer, GCompOpAssign);
			layer_mark_dirty(bitmap_layer_get_layer(gradient_layer));
		} else {
			layer_set_frame(bitmap_layer_get_layer(gradient_layer), GRect(0, 0, 144, 168));
			bitmap_layer_set_compositing_mode(gradient_layer, GCompOpSet);
		}
		#else
			layer_set_frame(bitmap_layer_get_layer(gradient_layer), GRect(gradient_x, gradient_y, 144, 168));
		#endif

		layer_set_hidden(bitmap_layer_get_layer(gradient_layer), (settings.gradient == 48) ? true : false);
		
	}
	
	updateComplications(&settings, day_text_layer, date_text_layer, day_buffer, dateText, dayDate_buffer, month_buffer, short_date_buffer, steps_buffer);
	updateGradient();

}

static void load_default_settings() {
	settings.hourlyVibe = 	0;
	settings.background = 	1;
	settings.gradient = 	48;
	settings.gradColor = 	GColorBlack;
	settings.comp1 = 		49;
	settings.comp2 = 		50;
	settings.hourColor = 	GColorWhite;
	settings.minuteColor = 	GColorWhite;
	settings.layout = 		48;
	settings.startTime = 	7;
    settings.endTime = 		22;
    settings.vibeStrong = 	false;
}

static void load_persistent_settings() {
	load_default_settings();
	persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {

    // Get the bitmap
    Tuple *size_tuple  = dict_find(iterator, MESSAGE_KEY_size);
    if(size_tuple){
        if(data_image)
            free(data_image);
            data_size = size_tuple->value->uint32;
            data_image = malloc(data_size);
    }	
	Tuple *image_tuple = dict_find(iterator, MESSAGE_KEY_chunk);
    Tuple *index_tuple = dict_find(iterator, MESSAGE_KEY_index);

    if (index_tuple && image_tuple) {
		
        int32_t index = index_tuple->value->int32;
        // APP_LOG(APP_LOG_LEVEL_INFO, "image received index=%ld size=%d", index, image_tuple->length);
        memcpy(data_image + index,&image_tuple->value->uint8,image_tuple->length);
        if(image_tuple->length < CHUNK_SIZE){

			// Clear the image
			if(image){
				gbitmap_destroy(image);
				image = NULL;
			}

            #ifdef PBL_COLOR
            	image = gbitmap_create_from_png_data(data_image, data_size);
            #else
            	image = gbitmap_create_with_data(data_image);
            #endif
			
			text_layer_set_text(info_text_layer, "");
            bitmap_layer_set_bitmap(image_layer, image);
            layer_mark_dirty(bitmap_layer_get_layer(image_layer));
        }
			
    }
	
	Tuple *message_tuple = dict_find(iterator, MESSAGE_KEY_message);
	if (message_tuple && message_tuple->value->cstring && strlen(message_tuple->value->cstring) > 0) {
		// If message_tuple is not empty, show the layer and set the text
		layer_set_hidden(text_layer_get_layer(info_text_layer), false);
		text_layer_set_text(info_text_layer, message_tuple->value->cstring);
	} else {
		// If message_tuple is empty or NULL, hide the layer
		layer_set_hidden(text_layer_get_layer(info_text_layer), true);
	}
	
	Tuple *background_tuple = dict_find(iterator, MESSAGE_KEY_background);
	background_tuple ? settings.background = (background_tuple->value->uint8) : false;	
	
	Tuple *hourlyVibe_tuple = dict_find(iterator, MESSAGE_KEY_hourlyVibe);
	hourlyVibe_tuple ? settings.hourlyVibe = atoi(hourlyVibe_tuple->value->cstring) : 0;

	Tuple *hourColor_tuple = dict_find(iterator, MESSAGE_KEY_hourColor);
	hourColor_tuple ? settings.hourColor = GColorFromHEX(hourColor_tuple->value->int32) : GColorWhite;

	Tuple *minuteColor_tuple = dict_find(iterator, MESSAGE_KEY_minuteColor);
	minuteColor_tuple ? settings.minuteColor = GColorFromHEX(minuteColor_tuple->value->int32) : GColorWhite;

	Tuple *gradColor_tuple = dict_find(iterator, MESSAGE_KEY_gradColor);
	gradColor_tuple ? settings.gradColor = GColorFromHEX(gradColor_tuple->value->int32) : GColorBlack;

	Tuple *gradient_tuple = dict_find(iterator, MESSAGE_KEY_gradient);
	gradient_tuple ? settings.gradient = (gradient_tuple->value->int32) : 48;

	Tuple *comp1_tuple = dict_find(iterator, MESSAGE_KEY_comp1);
	comp1_tuple ? settings.comp1 = (comp1_tuple->value->int32) : 48;

	Tuple *comp2_tuple = dict_find(iterator, MESSAGE_KEY_comp2);
	comp2_tuple ? settings.comp2 = (comp2_tuple->value->int32) : 48;

	Tuple *layout_tuple = dict_find(iterator, MESSAGE_KEY_layout);
	layout_tuple ? settings.layout = (layout_tuple->value->int32) : 48;

	Tuple *startTime_tuple = dict_find(iterator, MESSAGE_KEY_startTime);
    startTime_tuple ? settings.startTime = (startTime_tuple->value->int32) : 7;

    Tuple *endTime_tuple = dict_find(iterator, MESSAGE_KEY_endTime);
    endTime_tuple ? settings.endTime = (endTime_tuple->value->int32) : 22;

	Tuple *vibeStrong_tuple = dict_find(iterator, MESSAGE_KEY_vibeStrong);
    vibeStrong_tuple ? settings.vibeStrong = atoi(vibeStrong_tuple->value->cstring) : 0;

	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));

	layer_mark_dirty(bitmap_layer_get_layer(image_layer));
	updateLayers();
    
}

#include "health.h"

// Local callback called by health module when steps update is available.
static void img999_on_health_update(int steps) {
	if (window) {
		char buffer[7] = {0};
		health_abbrev_steps(steps, buffer);
		snprintf(steps_buffer, sizeof(steps_buffer), "%s", buffer);
	} else {
		snprintf(steps_buffer, sizeof(steps_buffer), "000");
	}
	updateComplications(&settings, day_text_layer, date_text_layer, day_buffer, dateText, dayDate_buffer, month_buffer, short_date_buffer, steps_buffer);
}
  
int is_active_time(int start_hour, int end_hour) {
    // Get current time
    time_t now = time(NULL);
    struct tm *current = localtime(&now);
    int current_hour = current->tm_hour; // 24-hour format (0-23)
    int current_min = current->tm_min;   // Minutes (0-59)

    // Convert times to total minutes since midnight (no minutes in input, so use 0)
    int start_total_min = start_hour * 60;
    int end_total_min = end_hour * 60;
    int current_total_min = current_hour * 60 + current_min;

    // Check if time range crosses midnight
    if (start_total_min <= end_total_min) {
        // No midnight crossover (e.g., 7 to 13 for 7 AM to 1 PM)
        // Return 1 if current time is within [start, end)
        return current_total_min >= start_total_min && current_total_min < end_total_min;
    } else {
        // Midnight crossover (e.g., 22 to 2 for 10 PM to 2 AM)
        // Return 1 if current time is after start or before end
        return current_total_min >= start_total_min || current_total_min < end_total_min;
    }
}

static void vibration_handler(void *context) {  
	if (loopCounter <= loopCount) {
		vibes_enqueue_custom_pattern(get_vibe_pattern());
		vibration_timer = app_timer_register(vibration_delay, vibration_handler, NULL);
		vibration_timer_active = true;
		loopCounter++;
	} else {
		loopCounter = 0;
		loopCount = 0;
		vibration_timer_active = false;
	}
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {

	static char date_buffer[6];
    strftime(date_buffer, sizeof(date_buffer), "%e", tick_time); // Format the rest of the date

	const char *suff;
	int date_suffix = atoi(date_buffer);
	switch (date_suffix)
	{
	case 1: case 21: case 31:
	  suff = "st";
	  break;
	case 2: case 22:
	  suff = "nd";
	  break;
	case 3: case 23:
	  suff = "rd";
	  break;
	default:
	  suff = "th";
	  break;
	}
  
	snprintf(dateText, sizeof(dateText), "%s%s", date_buffer, suff);

    // Determine the time format (24-hour or 12-hour) for hours
    static char s_hour_buffer[8] = {0};
    strftime(s_hour_buffer, sizeof(s_hour_buffer), clock_is_24h_style() ? "%H" : "%I", tick_time);

    // Extract minutes
    static char s_minute_buffer[8] = {0};
    strftime(s_minute_buffer, sizeof(s_minute_buffer), "%M", tick_time);

	// Get the full day name and store it in day_buffer
	strftime(day_buffer, sizeof(day_buffer), "%a", tick_time);

	// Convert the day name in day_buffer to uppercase
	for (int i = 0; day_buffer[i] != '\0'; i++) {
		day_buffer[i] = toupper((unsigned char)day_buffer[i]);
	}

	// Convert the day of the month to an integer
    int day_of_month = atoi(date_buffer);

	// Format the abbreviated day name (first two characters of day_buffer) and date into dayDate_buffer
    snprintf(dayDate_buffer, sizeof(dayDate_buffer), "%.2s, %02d", day_buffer, day_of_month);

	 // Get the short month name and store it in month_buffer
	 strftime(month_buffer, sizeof(month_buffer), "%b", tick_time);

	 // Convert the month name in month_buffer to uppercase
	 for (int i = 0; month_buffer[i] != '\0'; i++) {
		 month_buffer[i] = toupper((unsigned char)month_buffer[i]);
	 }

	 snprintf(short_date_buffer, sizeof(short_date_buffer), "%d|%d", 
         tick_time->tm_mon + 1, tick_time->tm_mday);

    // Update the text layers
    text_layer_set_text(hour_text_layer, s_hour_buffer);  // Set only the hours
    text_layer_set_text(minute_text_layer, s_minute_buffer);  // Set only the minutes
    int minutes = tick_time->tm_min;
	int seconds = tick_time->tm_sec;

    // Vibrate if enabled
	bool active = is_active_time(settings.startTime, settings.endTime);
	if(!vibration_timer_active && active && seconds == 0) {

	if (settings.hourlyVibe == 2 && minutes == 0) {
		loopCount = 0;
		vibration_timer = app_timer_register(vibration_delay, vibration_handler, NULL);
		vibration_timer_active = true;
	} else if (settings.hourlyVibe == 1 && minutes%15 == 0) {
		switch (minutes) {
			case 0:
				loopCount = 3;
				break;
			case 15:
				loopCount = 0;
				break;
			case 30:
				loopCount = 1;
				break;
			case 45:
				loopCount = 2;
				break;
			default:
				loopCount = 0;
				break;
		}
		vibration_timer = app_timer_register(vibration_delay, vibration_handler, NULL);
		vibration_timer_active = true;
	}
	
}
}

uint8_t adjustHeight(int16_t offset_start, int16_t offset_end, int16_t height) {
    int16_t new_height = 168 - height;

    if (new_height > offset_end) {
        return offset_end;
    } else if (new_height < offset_start) {
        return offset_start;
    }

    return new_height;
}

uint8_t adjustWidth(int16_t offset_start, int16_t offset_end, int16_t height) {
	int16_t new_height = 168 - height;
    int16_t new_width = 144 - new_height;
    
    if (new_width < offset_end) {
        return offset_end;
    } else if (new_width > offset_start) {
        return offset_start;
    } else {
		return (offset_start - new_width < offset_end) ? offset_end : offset_start - new_width;
	}
}

#ifndef PBL_PLATFORM_APLITE
static void unobstructed_change(AnimationProgress _p, void *window) {
	// APP_LOG(APP_LOG_LEVEL_INFO, "unobstructed_change");
	Layer *layer  = window_get_root_layer((Window *) window);
	GRect bounds = layer_get_bounds(layer);
	GRect unObsBounds = layer_get_unobstructed_bounds(layer);
	isObstructed = !grect_equal(&bounds, &unObsBounds);
	// int16_t obstruction_height = bounds.size.h - unObsBounds.size.h;
	// APP_LOG(APP_LOG_LEVEL_INFO, "Unobstructed Height: %d", obstruction_height);

	updateLayers();

	if  (settings.layout == 48) {
		GRect hour = layer_get_frame(text_layer_get_layer(hour_text_layer));
		hour.origin.y = (isObstructed) ? 6 : 2;
		layer_set_frame(text_layer_get_layer(hour_text_layer), hour);

		GRect minute = layer_get_frame(text_layer_get_layer(minute_text_layer));
		minute.origin.x = (isObstructed) ? 50 : 3;
		minute.origin.y = adjustHeight(minute_start_L, minute_end_L, unObsBounds.size.h);
		layer_set_frame(text_layer_get_layer(minute_text_layer), minute);
	} else if (settings.layout == 49) {
		GRect minute = layer_get_frame(text_layer_get_layer(minute_text_layer));
		minute.origin.y = adjustHeight(minute_start, minute_end, unObsBounds.size.h);
		layer_set_frame(text_layer_get_layer(minute_text_layer), minute);

		GRect hour = layer_get_frame(text_layer_get_layer(hour_text_layer));
		hour.origin.x = adjustWidth(hour_start, hour_end, unObsBounds.size.h);
		hour.origin.y = (isObstructed) ? 6 : 2;
		layer_set_frame(text_layer_get_layer(hour_text_layer), hour);
		
	} else if (settings.layout == 50 || settings.layout == 51) {
		GRect hour = layer_get_frame(text_layer_get_layer(hour_text_layer));
		GRect minute = layer_get_frame(text_layer_get_layer(minute_text_layer));

		if (!isObstructed) {
			hour.origin.y = hour_y;
			minute.origin.y = minute_y;
		} else {
			hour.origin.y = adjustHeight(hour_y, horiz_time_end, unObsBounds.size.h);
			minute.origin.y = adjustHeight(minute_y, horiz_time_end, unObsBounds.size.h);
		}

		layer_set_frame(text_layer_get_layer(hour_text_layer), hour);
		layer_set_frame(text_layer_get_layer(minute_text_layer), minute);
	} 

}
#endif

static void window_load(Window *window) {

	window_set_background_color(window, GColorBlack);

	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	image_layer = bitmap_layer_create(GRect(-18, -6, 180, 180));

	bitmap_layer_set_bitmap(image_layer, image);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);

	info_text_layer = text_layer_create(GRect(0, bounds.size.h - 92, bounds.size.w, 16));
	text_layer_set_font(info_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(info_text_layer, GTextAlignmentCenter);
	text_layer_set_background_color(info_text_layer, GColorBlack);
	text_layer_set_text_color(info_text_layer, GColorWhite);

	apply_layout_from_settings();

	hour_text_layer = text_layer_create(GRect(hour_x, hour_y, hour_width, hour_height));
	text_layer_set_background_color(hour_text_layer, GColorClear);
	text_layer_set_text_color(hour_text_layer, settings.hourColor);

	minute_text_layer = text_layer_create(GRect(minute_x, minute_y, minute_width, minute_height));
	text_layer_set_background_color(minute_text_layer, GColorClear);
	text_layer_set_text_color(minute_text_layer, settings.minuteColor);

	date_text_layer = text_layer_create(GRect(comp2_x, comp2_y, comp2_width, comp2_height));
	text_layer_set_background_color(date_text_layer, GColorClear);
	text_layer_set_text_color(date_text_layer, settings.minuteColor);
	text_layer_set_font(date_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	day_text_layer = text_layer_create(GRect(comp1_x, comp1_y, comp1_width, comp1_height));
	text_layer_set_background_color(day_text_layer, GColorClear);
	text_layer_set_text_color(day_text_layer, settings.hourColor);
	text_layer_set_font(day_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	#ifdef PBL_BW
		gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_0);
	#endif

	#ifdef PBL_BW
		gradient_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	#else
		gradient_layer = bitmap_layer_create(GRect((!settings.layout) ? -94 : 93, 0, 144, 168));
	#endif

	bitmap_layer_set_bitmap(gradient_layer, gradient);
	#ifdef PBL_BW
		if (settings.gradient == 49) {
			bitmap_layer_set_compositing_mode(gradient_layer, GCompOpAssign);
		} else {
			bitmap_layer_set_compositing_mode(gradient_layer, GCompOpSet);
		}
	#else
		bitmap_layer_set_compositing_mode(gradient_layer, GCompOpSet);
	#endif

	#ifdef PBL_COLOR
		gbitmap_fill_all_except(GColorClear, settings.gradColor, true, gradient, gradient_layer);
	#endif

	if (isObstructed) {
		layer_set_hidden(bitmap_layer_get_layer(gradient_layer), false);
	} else {
		layer_set_hidden(bitmap_layer_get_layer(gradient_layer), (settings.gradient == 48) ? true : false);
	}

	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));
	layer_add_child(window_layer, bitmap_layer_get_layer(gradient_layer));
	layer_add_child(window_layer, text_layer_get_layer(hour_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(minute_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(date_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(day_text_layer));
	layer_add_child(window_layer, text_layer_get_layer(info_text_layer));
	
	#ifndef PBL_PLATFORM_APLITE
		UnobstructedAreaHandlers unObsHeight = { 0, unobstructed_change, 0 };
		unobstructed_area_service_subscribe(unObsHeight, window);
		unobstructed_change(0, window);
	#endif

}

static void window_unload(Window *window) {
	if(data_image){
		free(data_image);
		data_image = NULL;
	}
	gbitmap_destroy_safe(&image);
	gbitmap_destroy_safe(&gradient);
	fonts_unload_custom_font(time_font);
	bitmap_layer_destroy(image_layer);
	bitmap_layer_destroy(gradient_layer);
	text_layer_destroy(hour_text_layer);
	text_layer_destroy(minute_text_layer);
	text_layer_destroy(date_text_layer);
	text_layer_destroy(day_text_layer);
	text_layer_destroy(info_text_layer);
	health_deinit();
	tick_timer_service_unsubscribe();
	app_message_deregister_callbacks();
	#ifndef PBL_PLATFORM_APLITE
		unobstructed_area_service_unsubscribe();
	#endif
}

static void init(void) {
	window = window_create();

	load_persistent_settings();

	app_message_register_inbox_received(inbox_received_callback);
	app_message_open(80000, 64);

	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	health_init(img999_on_health_update);
	window_stack_push(window, true);
	tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);	

	time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);
    tick_handler(tick_time, MINUTE_UNIT);
	
}

static void deinit(void) {
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}