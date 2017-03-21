#include <pebble.h>
#include <math.h>

#define PI 3.141592

static Window *s_main_window;
static Layer *s_seconds_layer;
static TextLayer *s_time_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_seconds(Layer *layer, GContext *ctx){
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	graphics_context_set_stroke_color(ctx, GColorRed);
	graphics_context_set_fill_color(ctx, GColorRed);
	double angle_1 = ((double)tick_time->tm_sec * 2.5 - 112.5) * PI / 180;
	double angle_2 = ((double)tick_time->tm_sec * 2.5 - 112.6) * PI / 180;
	double angle_3 = ((double)tick_time->tm_sec * 2.5 - 112.4) * PI / 180;
	double angle_4 = ((double)tick_time->tm_sec * 2.5 - 112.7) * PI / 180;
	double angle_5 = ((double)tick_time->tm_sec * 2.5 - 112.8) * PI / 180;
	double xpos_1 = sin(angle_1) * 69.0 + 72.0;
	double ypos_1 = -cos(angle_1) * 69.0 + 72.0;
	double xpos_2 = sin(angle_2) * 41.0 + 72.0;
	double ypos_2 = -cos(angle_2) * 41.0 + 72.0;
	double xpos_3 = sin(angle_3) * 41.0 + 72.0;
	double ypos_3 = -cos(angle_3) * 41.0 + 72.0;
	double xpos_4 = sin(angle_4) * 41.0 + 72.0;
	double ypos_4 = -cos(angle_4) * 41.0 + 72.0;
	double xpos_5 = sin(angle_5) * 41.0 + 72.0;
	double ypos_5 = -cos(angle_5) * 41.0 + 72.0;
	graphics_draw_line(ctx, GPoint(xpos_1,ypos_1), GPoint(xpos_2,ypos_2));
	graphics_draw_line(ctx, GPoint(xpos_1,ypos_1), GPoint(xpos_3,ypos_3));
	graphics_draw_line(ctx, GPoint(xpos_1,ypos_1), GPoint(xpos_4,ypos_4));
	graphics_draw_line(ctx, GPoint(xpos_1,ypos_1), GPoint(xpos_5,ypos_5));
}

static void update_time(){
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	static char buffer[] = "00:00";
	if(clock_is_24h_style() == true){
		strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
	}else{
		strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
	}
	text_layer_set_text(s_time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
	update_time();
	layer_mark_dirty(s_seconds_layer);
}

static void main_window_load(Window *window){
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_base);
	s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
	s_time_layer = text_layer_create(GRect(22,72,100,40));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_text(s_time_layer, "00:00");
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	s_seconds_layer = layer_create(GRect(0,20,144,144));
	layer_set_update_proc(s_seconds_layer, update_seconds);
	layer_add_child(window_get_root_layer(window), s_seconds_layer);
	update_time();
}

static void main_window_unload(Window *window){
	layer_destroy(s_seconds_layer);
	text_layer_destroy(s_time_layer);
	gbitmap_destroy(s_background_bitmap);
	bitmap_layer_destroy(s_background_layer);
}

static void init(){
	s_main_window = window_create();
	window_set_background_color(s_main_window, GColorDarkGray);
	window_set_window_handlers(s_main_window, (WindowHandlers){
		.load = main_window_load,
		.unload = main_window_unload
	});
	window_stack_push(s_main_window, true);
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit(){
	window_destroy(s_main_window);
}

int main(void){
	init();
	app_event_loop();
	deinit();
}
