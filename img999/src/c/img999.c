#include <pebble.h>
#include <ctype.h>
#include "img999.h"
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
static char 			dateText[6];
static char 			day_buffer[4];
static char 			dayDate_buffer[8];
static char 			month_buffer[4];
static char 			steps_buffer[8];
static char 			short_date_buffer[8];
static bool 			isObstructed = false;
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
bool 					health_is_available();
static bool 			s_health_available;
int 					health_get_metric_sum(HealthMetric metric);
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

const VibePattern vibe_pattern = {
	.durations = (uint32_t []) {50, 20, 50, 300},
		.num_segments = 4
};

static void updateLayout() {

	switch (settings.layout) {
		case 48:
			hour_x = 3;
			hour_y = 2;
			hour_width = 50;
			hour_height = 80;

			minute_x = 3;
			minute_y = 62;
			minute_width = 50;
			minute_height = 80;

			comp1_x = 1;
			comp1_y = (settings.comp1 == 51) ? 8 : -2;
			comp1_width = 48;
			comp1_height = 26;

			comp2_x = 1;
			comp2_y = (settings.comp2 == 51) ? 142 : 136;
			comp2_width = 48;
			comp2_height = 26;

			gradient_x = -94;
			gradient_y = 0;
			break;
		case 49:
			hour_x = 96;
			hour_y = 2;
			hour_width = 50;
			hour_height = 80;

			minute_x = 96;
			minute_y = 62;
			minute_width = 50;
			minute_height = 80;

			comp1_x = 94;
			comp1_y = (settings.comp1 == 51) ? 8 : -2;
			comp1_width = 48;
			comp1_height = 26;

			comp2_x = 94;
			comp2_y = (settings.comp2 == 51) ? 142 : 136;
			comp2_width = 48;
			comp2_height = 26;

			gradient_x = 93;
			gradient_y = 0;
			break;
		case 50:
			hour_x = 44;
			hour_y = -4;
			hour_width = 30;
			hour_height = 30;

			minute_x = 75;
			minute_y = -4;
			minute_width = 30;
			minute_height = 30;

			comp1_x = 3;
			comp1_y = (settings.comp1 == 51) ? 4 : -3;
			comp1_width = 40;
			comp1_height = 26;

			comp2_x = 100;
			comp2_y = (settings.comp2 == 51) ? 4 : -3;
			comp2_width = 40;
			comp2_height = 26;

			gradient_x = 0;
			gradient_y = -138;
			break;
		case 51:
			hour_x = 44;
			hour_y = 136;
			hour_width = 30;
			hour_height = 30;

			minute_x = 75;
			minute_y = 136;
			minute_width = 30;
			minute_height = 30;

			comp1_x = 3;
			comp1_y = (settings.comp1 == 51) ? 144 : 138;
			comp1_width = 40;
			comp1_height = 26;

			comp2_x = 100;
			comp2_y = (settings.comp2 == 51) ? 144 : 138;
			comp2_width = 40;
			comp2_height = 26;
			
			gradient_x = 0;
			gradient_y = 138;
			break;
		default:
			hour_x = 3;
			hour_y = 2;
			hour_width = 50;
			hour_height = 80;

			minute_x = 3;
			minute_y = 62;
			minute_width = 50;
			minute_height = 80;

			comp1_x = 1;
			comp1_y = (settings.comp1 == 51) ? 8 : -2;
			comp1_width = 48;
			comp1_height = 26;

			comp2_x = 1;
			comp2_y = (settings.comp2 == 51) ? 142 : 136;
			comp2_width = 48;
			comp2_height = 26;

			gradient_x = -94;
			gradient_y = 0;
		break;
	}	

}

static void updateGradient() {

	if (settings.gradient == 49) {
		bitmap_layer_set_background_color(gradient_layer, settings.gradColor);
	} else {
		bitmap_layer_set_background_color(gradient_layer, GColorClear);
	}	

	if (gradient != NULL) {
		gbitmap_destroy(gradient);
		gradient = NULL;
	}

	switch (settings.gradient) {
		case 50:
			#ifdef PBL_PLATFORM_DIORITE
				gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_2);
			#elif PBL_PLATFORM_APLITE
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
			#ifdef PBL_PLATFORM_DIORITE
				gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_3);
			#elif PBL_PLATFORM_APLITE
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

static void updateComplications() {

	text_layer_set_font(day_text_layer, fonts_get_system_font((settings.comp1 == 51) ? FONT_KEY_GOTHIC_14_BOLD : FONT_KEY_GOTHIC_24_BOLD));
	text_layer_set_font(date_text_layer, fonts_get_system_font((settings.comp2 == 51) ? FONT_KEY_GOTHIC_14_BOLD : FONT_KEY_GOTHIC_24_BOLD));

	switch (settings.comp2) {
		case 49:
			text_layer_set_text(date_text_layer, day_buffer);
			break;
		case 50:
			text_layer_set_text(date_text_layer, dateText);
			break;
		case 51:
			text_layer_set_text(date_text_layer, dayDate_buffer);
			break;
		case 52:
			text_layer_set_text(date_text_layer, month_buffer);
			break;
		case 53:
			text_layer_set_text(date_text_layer, short_date_buffer);
			break;
		case 54:
			text_layer_set_text(date_text_layer, steps_buffer);
			break;
		default:
			layer_set_hidden(text_layer_get_layer(date_text_layer), true);
			break;
	}

	switch (settings.comp1) {
		case 49:
			text_layer_set_text(day_text_layer, day_buffer);
			break;
		case 50:
			text_layer_set_text(day_text_layer, dateText);
			break;
		case 51:
			text_layer_set_text(day_text_layer, dayDate_buffer);
			break;
		case 52:
			text_layer_set_text(day_text_layer, month_buffer);
			break;
		case 53:
			text_layer_set_text(day_text_layer, short_date_buffer);
			break;
		case 54:
			text_layer_set_text(day_text_layer, steps_buffer);
			break;
		default:
			layer_set_hidden(text_layer_get_layer(day_text_layer), true);
			break;
	}

}

static void updateLayers() {
	updateLayout();

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
			if (gradient != NULL) {
				gbitmap_destroy(gradient);
				gradient = NULL;
			}

			#ifdef PBL_PLATFORM_DIORITE
				gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_2);
			#endif

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
		
		#ifdef PBL_PLATFORM_APLITE
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
		
		#ifdef PBL_PLATFORM_APLITE
		layer_set_frame(bitmap_layer_get_layer(gradient_layer), GRect(0, 0, 144, 168));
		#else
			layer_set_frame(bitmap_layer_get_layer(gradient_layer), GRect(gradient_x, gradient_y, 144, 168));
		#endif

		layer_set_hidden(bitmap_layer_get_layer(gradient_layer), (settings.gradient == 48) ? true : false);
		
	}
	
	updateComplications();
	updateGradient();

}

static void load_default_settings() {
	settings.hourlyvibe = 	1;
	settings.background = 	1;
	settings.gradient = 	48;
	settings.gradColor = 	GColorBlack;
	settings.comp1 = 		49;
	settings.comp2 = 		50;
	settings.hourColor = 	GColorWhite;
	settings.minuteColor = 	GColorWhite;
	settings.layout = 		48;
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
	
	Tuple *hourlyvibe_tuple = dict_find(iterator, MESSAGE_KEY_hourlyvibe);
	hourlyvibe_tuple ? settings.hourlyvibe = (hourlyvibe_tuple->value->uint8) : false;

	Tuple *hourColor_tuple = dict_find(iterator, MESSAGE_KEY_hourColor);
	#ifdef PBL_COLOR
		hourColor_tuple ? settings.hourColor = GColorFromHEX(hourColor_tuple->value->int32) : GColorWhite;
	#else
		settings.hourColor = (hourColor_tuple && strlen(hourColor_tuple->value->cstring) < 8) 
			? GColorBlack 
			: GColorWhite;
	#endif

	Tuple *minuteColor_tuple = dict_find(iterator, MESSAGE_KEY_minuteColor);
	#ifdef PBL_COLOR
		minuteColor_tuple ? settings.minuteColor = GColorFromHEX(minuteColor_tuple->value->int32) : GColorWhite;
	#else
		settings.minuteColor = (minuteColor_tuple && strlen(minuteColor_tuple->value->cstring) < 8) 
			? GColorBlack 
			: GColorWhite;
	#endif

	Tuple *gradColor_tuple = dict_find(iterator, MESSAGE_KEY_gradColor);
	#ifdef PBL_COLOR
		gradColor_tuple ? settings.gradColor = GColorFromHEX(gradColor_tuple->value->int32) : GColorBlack;
	#else
		settings.gradColor = (gradColor_tuple && strlen(gradColor_tuple->value->cstring) < 8) 
			? GColorBlack 
			: GColorWhite;
	#endif

	Tuple *gradient_tuple = dict_find(iterator, MESSAGE_KEY_gradient);
	gradient_tuple ? settings.gradient = (gradient_tuple->value->int32) : 48;

	Tuple *comp1_tuple = dict_find(iterator, MESSAGE_KEY_comp1);
	comp1_tuple ? settings.comp1 = (comp1_tuple->value->int32) : 48;

	Tuple *comp2_tuple = dict_find(iterator, MESSAGE_KEY_comp2);
	comp2_tuple ? settings.comp2 = (comp2_tuple->value->int32) : 48;

	Tuple *layout_tuple = dict_find(iterator, MESSAGE_KEY_layout);
	layout_tuple ? settings.layout = (layout_tuple->value->int32) : 48;

	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));

	layer_mark_dirty(bitmap_layer_get_layer(image_layer));
	updateLayers();
	
}

bool health_is_available() {
	return s_health_available;
  }
  
int health_get_metric_sum(HealthMetric metric) {
	HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric,
	  time_start_of_today(), time(NULL));
	if(mask == HealthServiceAccessibilityMaskAvailable) {
	  return (int)health_service_sum_today(metric);
	} else {
	  return 0;
	}
  }

void abbrevSteps(int num, char* result) {
    for (int i = 0; i < 7; i++) {
        result[i] = '\0';
    }
    if (num == 0) {
        result[0] = '0';
        return;
    }
    char temp[10];
    int digitCount = 0;
    int tempNum = num;
    while (tempNum > 0 && digitCount < 10) {
        temp[digitCount] = (tempNum % 10) + '0';
        tempNum /= 10;
        digitCount++;
    }
    if (digitCount <= 3) {
        for (int i = 0; i < digitCount; i++) {
            result[i] = temp[digitCount - 1 - i];
        }
        return;
    }
    if (digitCount == 4) {
        result[0] = temp[3];
        if (temp[2] != '0') {
            result[1] = '.';
            result[2] = temp[2];
            result[3] = 'k';
        } else {
            result[1] = 'k';
        }
        return;
    }
    if (digitCount == 5) {
        result[0] = temp[4];
        result[1] = temp[3];
        if (temp[2] != '0') {
            result[2] = '.';
            result[3] = temp[2];
            result[4] = 'k';
        } else {
            result[2] = 'k';
        }
        return;
    }
}

static void prv_on_health_data(HealthEventType type, void *context) {
	if(health_is_available() && window) {
		char buffer[7] = {0};
		abbrevSteps(health_get_metric_sum(HealthMetricStepCount), buffer);
		snprintf(steps_buffer, sizeof(steps_buffer), "%s", buffer);
	} else {
		snprintf(steps_buffer, sizeof(steps_buffer), "000");
	}
	updateComplications();
}

void health_init() {
	s_health_available = health_service_events_subscribe(prv_on_health_data, NULL);
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

    // Handle sleep mode logic
    // int hour = tick_time->tm_hour;
    int seconds = tick_time->tm_sec;
    int minutes = tick_time->tm_min;

    // Vibrate every hour if enabled
    if (minutes == 0 && seconds == 0 && settings.hourlyvibe == 1) {
        vibes_enqueue_custom_pattern(vibe_pattern);
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

	updateLayout();

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

	#ifdef PBL_PLATFORM_DIORITE
		gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_2);
	#else
		gradient = gbitmap_create_with_resource(RESOURCE_ID_GRADIENT_0);
	#endif

	#ifdef PBL_PLATFORM_APLITE
		gradient_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	#else
		gradient_layer = bitmap_layer_create(GRect((!settings.layout) ? -94 : 93, 0, 144, 168));
	#endif

	bitmap_layer_set_bitmap(gradient_layer, gradient);
	bitmap_layer_set_compositing_mode(gradient_layer, GCompOpSet);

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
	if(image){
		gbitmap_destroy(image);
		image = NULL;
	}
	if(gradient){
		gbitmap_destroy(gradient);
		gradient = NULL;
	}
	fonts_unload_custom_font(time_font);
	bitmap_layer_destroy(image_layer);
	bitmap_layer_destroy(gradient_layer);
	text_layer_destroy(hour_text_layer);
	text_layer_destroy(minute_text_layer);
	text_layer_destroy(date_text_layer);
	text_layer_destroy(day_text_layer);
	text_layer_destroy(info_text_layer);
	health_service_events_unsubscribe();
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
	health_init();
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