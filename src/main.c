#include <pebble.h>
#include <ctype.h>
	
static Window *s_main_window;
static GBitmap *s_bitmap;
static GBitmap *battery_charge_image;
static GBitmap *bt_good_image;
static GBitmap *bt_bad_image;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static BitmapLayer *s_bitmap_layer;
static BitmapLayer *battery_charge_image_layer;
static BitmapLayer *bt_good_image_layer;
static BitmapLayer *bt_bad_image_layer;
static GFont s_time_font;
static GFont s_date_font;
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

void update_bluetooth_status(bool connected) {

	layer_set_hidden(bitmap_layer_get_layer(bt_bad_image_layer), connected);
	layer_set_hidden(bitmap_layer_get_layer(bt_good_image_layer), !connected);	

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

	// Good Bluetooth Image
	bt_good_image = gbitmap_create_with_resource(RESOURCE_ID_BT_GOOD);
	
	bt_good_image_layer = bitmap_layer_create(GRect(5, 8, 7, 13));
	bitmap_layer_set_bitmap(bt_good_image_layer, bt_good_image);
	bitmap_layer_set_compositing_mode(bt_good_image_layer, GCompOpSet);
	bitmap_layer_set_alignment(bt_good_image_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(bt_good_image_layer));

	// Bad Bluetooth Image
	bt_bad_image = gbitmap_create_with_resource(RESOURCE_ID_BT_BAD);
	
	bt_bad_image_layer = bitmap_layer_create(GRect(5, 8, 7, 13));
	bitmap_layer_set_bitmap(bt_bad_image_layer, bt_bad_image);
	bitmap_layer_set_compositing_mode(bt_bad_image_layer, GCompOpSet);
	bitmap_layer_set_alignment(bt_bad_image_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(bt_bad_image_layer));
	
}

static void main_window_unload(Window *window) {
	
	// Destroy layers.
	layer_destroy(battery_status_layer);
	text_layer_destroy(s_time_layer);
	text_layer_destroy(s_date_layer);
	bitmap_layer_destroy(s_bitmap_layer);
	bitmap_layer_destroy(battery_charge_image_layer);
	bitmap_layer_destroy(bt_good_image_layer);
	bitmap_layer_destroy(bt_bad_image_layer);
	
	// Destroy bitmaps
	gbitmap_destroy(s_bitmap);
	gbitmap_destroy(battery_charge_image);
	gbitmap_destroy(bt_bad_image);
	gbitmap_destroy(bt_good_image);

	// Destroy fonts
  	fonts_unload_custom_font(s_time_font);
  	fonts_unload_custom_font(s_date_font);

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	
	update_time();

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
	update_bluetooth_status(bluetooth_connection_service_peek());

	// Subscribe to bluetooth, battery, and time updates.
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	bluetooth_connection_service_subscribe(update_bluetooth_status);
	battery_state_service_subscribe(update_battery_display);
}

static void deinit() {
	
	window_destroy(s_main_window);

}

int main() {

	init();
	app_event_loop();
	deinit();

}