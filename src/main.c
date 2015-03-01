#include <pebble.h>
#include <ctype.h>

#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_main_window;
static GBitmap *s_bitmap;
static GBitmap *battery_charge_image;
static GBitmap *wt_none;
static GBitmap *wt_fog;
static GBitmap *wt_rain;
static GBitmap *wt_snow;
static GBitmap *wt_cloud;
static GBitmap *wt_partly;
static GBitmap *wt_sun;
static GBitmap *wt_thunder;
static GBitmap *wt_wind;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_temp_layer;
static BitmapLayer *s_bitmap_layer;
static BitmapLayer *battery_charge_image_layer;
static BitmapLayer *wt_condition;
static GFont s_time_font;
static GFont s_date_font;
static GFont s_temp_font;
static BatteryChargeState old_charge_state;
static Layer *battery_status_layer;

char *upcase(char *str) {
	
    char *s = str;

    while (*s) {
        *s++ = toupper((int)*s);
    }

    return str;
	
}

void update_battery_display(BatteryChargeState charge_state) {

	old_charge_state = charge_state;
	layer_mark_dirty(battery_status_layer);

}

void battery_status_layer_update(Layer* layer, GContext* ctx) {

	graphics_context_set_fill_color(ctx, GColorScreaminGreen);
	graphics_fill_rect(ctx, GRect(0, 0, old_charge_state.charge_percent*14/100, 5), 0, 0);

}

static void update_time() {

	// Get a tm structure
	time_t temp = time(NULL); 
	struct tm *tick_time = localtime(&temp);
 
	// Create a long-lived buffer
	static char buffer[] = "00:00";
	static char date[] = "MON 01 JAN";
 
	// Write the current hours and minutes into the buffer
	if (clock_is_24h_style() == true) {
		// Use 24 hour format
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	} else {
		// Use 12 hour format
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
	}
	
	strftime(date, sizeof("MON 01 JAN"), "%a %e %b", tick_time);
 
	// Display this time on the TextLayer
	text_layer_set_text(s_time_layer, buffer);
	text_layer_set_text(s_date_layer, upcase(date));
	
}

static void main_window_load(Window *window) {
	
	Layer *window_layer = window_get_root_layer(window);

	// Create Logo Layer
	s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO);

	s_bitmap_layer = bitmap_layer_create(GRect(0, 28, 144, 68));
	bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
	bitmap_layer_set_compositing_mode(s_bitmap_layer, GCompOpSet);
	bitmap_layer_set_alignment(s_bitmap_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));

	// Time
	s_time_layer = text_layer_create(GRect(0, 100, 144, 50));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorDarkGray);
	text_layer_set_text(s_time_layer, "00:00");

	s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_UBUNTU_REGULAR_36));

	text_layer_set_font(s_time_layer, s_time_font);
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));

	// Date
	s_date_layer = text_layer_create(GRect(0, 136, 144, 25));
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorDarkGray);
	text_layer_set_text(s_date_layer, "MON 01 JAN");

	s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_UBUNTU_BOLD_18));

	text_layer_set_font(s_date_layer, s_date_font);
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));

	// Battery Body
	battery_charge_image = gbitmap_create_with_resource(RESOURCE_ID_BATTERY_BODY);
	
	battery_charge_image_layer = bitmap_layer_create(GRect(119, 10, 19, 9));
	bitmap_layer_set_bitmap(battery_charge_image_layer, battery_charge_image);
	bitmap_layer_set_compositing_mode(battery_charge_image_layer, GCompOpSet);
	bitmap_layer_set_alignment(battery_charge_image_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(battery_charge_image_layer));

	// Battery Load
	battery_status_layer = layer_create(GRect(121, 12, 14, 5));
	layer_add_child(window_get_root_layer(window), battery_status_layer);
	layer_set_update_proc(battery_status_layer, battery_status_layer_update);

	// Weather
	wt_none = gbitmap_create_with_resource(RESOURCE_ID_DISCONNECT);
	wt_fog = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_FOG);
	wt_rain = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_RAIN);
	wt_snow = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_SNOW);
	wt_cloud = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_CLOUD);
	wt_partly = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_PARTLY_CLOUDY);
	wt_sun = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_SUN);
	wt_thunder = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_THUNDER);
	wt_wind = gbitmap_create_with_resource(RESOURCE_ID_WEATHER_WIND);	

	wt_condition = bitmap_layer_create(GRect(5, 6, 18, 18));
	bitmap_layer_set_bitmap(wt_condition, wt_none);
	bitmap_layer_set_compositing_mode(wt_condition, GCompOpSet);
	bitmap_layer_set_alignment(wt_condition, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(wt_condition));
	
	// Temperature
	s_temp_layer = text_layer_create(GRect(25, 6, 40, 25));
	text_layer_set_background_color(s_temp_layer, GColorClear);
	text_layer_set_text_color(s_temp_layer, GColorDarkGray);
	text_layer_set_text_alignment(s_temp_layer, GTextAlignmentLeft);
	text_layer_set_text(s_temp_layer, "0\u00B0");

	// Create second custom font, apply it and add to Window
	s_temp_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_UBUNTU_BOLD_14));
	text_layer_set_font(s_temp_layer, s_temp_font);
	layer_add_child(window_layer, text_layer_get_layer(s_temp_layer));
		
}

static void main_window_unload(Window *window) {
	
	// Destroy layers.
	layer_destroy(battery_status_layer);
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_temp_layer);
	bitmap_layer_destroy(s_bitmap_layer);
	bitmap_layer_destroy(battery_charge_image_layer);
	bitmap_layer_destroy(wt_condition);
	
	// Destroy bitmaps
	gbitmap_destroy(s_bitmap);
	gbitmap_destroy(battery_charge_image);
	gbitmap_destroy(wt_none);
	gbitmap_destroy(wt_fog);
	gbitmap_destroy(wt_rain);
	gbitmap_destroy(wt_snow);
	gbitmap_destroy(wt_partly);
	gbitmap_destroy(wt_sun);
	gbitmap_destroy(wt_thunder);
	gbitmap_destroy(wt_wind);

	// Destroy fonts
  	fonts_unload_custom_font(s_time_font);
  	fonts_unload_custom_font(s_date_font);
	fonts_unload_custom_font(s_temp_font);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	
	update_time();
	
	// Get weather update every 30 minutes
	if(tick_time->tm_min % 30 == 0) {
		// Begin dictionary
		DictionaryIterator *iter;
		app_message_outbox_begin(&iter);

		// Add a key-value pair
		dict_write_uint8(iter, 0, 0);

		// Send the message!
		app_message_outbox_send();
	}

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
	
	// Store incoming information
	static char temperature_buffer[8];
	static int conditions_buffer;
	static char weather_layer_buffer[32];

	// Read first item
	Tuple *t = dict_read_first(iterator);

	// For all items
	while(t != NULL) {
		// Which key was received?
		switch(t->key) {
			case KEY_TEMPERATURE:
				snprintf(temperature_buffer, sizeof(temperature_buffer), "%d\u00B0", (int)t->value->int32);
				break;
			case KEY_CONDITIONS:
				conditions_buffer = (int)t->value->int32;
				break;
			default:
				APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
				break;
		}

		// Look for next item
		t = dict_read_next(iterator);
	}

	// Display Temperature
	snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
	text_layer_set_text(s_temp_layer, weather_layer_buffer);
	
	APP_LOG(APP_LOG_LEVEL_ERROR, "%d is value for conditions", conditions_buffer);
	
	if (conditions_buffer >= 200 && conditions_buffer < 300) { // Thunderstorm
		bitmap_layer_set_bitmap(wt_condition, wt_thunder);
	} else if (conditions_buffer >= 300 && conditions_buffer < 600) { // Rainy
		bitmap_layer_set_bitmap(wt_condition, wt_rain);
	} else if (conditions_buffer >= 600 && conditions_buffer < 700) { // Snow
		bitmap_layer_set_bitmap(wt_condition, wt_snow);
	} else if (conditions_buffer >= 700 && conditions_buffer < 800) { // Foggy
		bitmap_layer_set_bitmap(wt_condition, wt_fog);
	} else if (conditions_buffer == 800) { // Sunny
		bitmap_layer_set_bitmap(wt_condition, wt_sun);
	} else if (conditions_buffer == 801) { // Partly Cloudy
		bitmap_layer_set_bitmap(wt_condition, wt_partly);
	} else if (conditions_buffer >= 802) { // Cloudy
		bitmap_layer_set_bitmap(wt_condition, wt_cloud);
	} else { // Disconnected
		bitmap_layer_set_bitmap(wt_condition, wt_none);
	}
	
}
 
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
	
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
	
}
 
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
	
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
	
}
 
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
	
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
	
}

static void init() {

	s_main_window = window_create();
	window_set_fullscreen(s_main_window, true);
	window_set_background_color(s_main_window, GColorLightGray);
	window_set_window_handlers(s_main_window, (WindowHandlers) {
    	.load = main_window_load,
    	.unload = main_window_unload,
	});
	
	window_stack_push(s_main_window, true);
	update_battery_display(battery_state_service_peek());

	// Subscribe to battery and time updates.
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	battery_state_service_subscribe(update_battery_display);
	
	// Register callbacks
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);

	// Open AppMessage
	app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

}

static void deinit() {
	
	window_destroy(s_main_window);

}

int main() {

	init();
	app_event_loop();
	deinit();

}