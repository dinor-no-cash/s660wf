#include <pebble.h>
#include <math.h>

#define PI 3.141592

static Window *s_main_window;
static Layer *s_seconds_layer;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_weekday_layer;
static Layer *s_bat_layer;
static Layer *s_conn_layer;
static TextLayer *s_step_layer;
static TextLayer *s_cals_layer;
static TextLayer *s_temp_layer;
static TextLayer *s_hum_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static GPath *s_bat_charge_path_ptr = NULL;
static const GPathInfo PATH_INFO_CHARGE = {.num_points = 6, .points = (GPoint[]){{40,2},{35,13},{39,13},{38,23},{43,12},{39,12}}};
static GPath *s_bat100_path_ptr = NULL;
static const GPathInfo PATH_INFO_100 = {.num_points = 4, .points = (GPoint[]){{0,0},{10,0},{16,3},{6,3}}};
static GPath *s_bat90_path_ptr = NULL;
static const GPathInfo PATH_INFO_90 = {.num_points = 4, .points = (GPoint[]){{8,5},{18,5},{23,8},{14,8}}};
static GPath *s_bat80_path_ptr = NULL;
static const GPathInfo PATH_INFO_80 = {.num_points = 4, .points = (GPoint[]){{16,10},{26,10},{29,13},{20,13}}};
static GPath *s_bat70_path_ptr = NULL;
static const GPathInfo PATH_INFO_70 = {.num_points = 4, .points = (GPoint[]){{22,15},{32,15},{35,18},{26,18}}};
static GPath *s_bat60_path_ptr = NULL;
static const GPathInfo PATH_INFO_60 = {.num_points = 4, .points = (GPoint[]){{27,20},{37,20},{39,23},{29,23}}};
static GPath *s_bat50_path_ptr = NULL;
static const GPathInfo PATH_INFO_50 = {.num_points = 4, .points = (GPoint[]){{31,25},{41,25},{42,28},{33,28}}};
static GPath *s_bat40_path_ptr = NULL;
static const GPathInfo PATH_INFO_40 = {.num_points = 4, .points = (GPoint[]){{34,30},{43,30},{44,33},{36,33}}};
static GPath *s_bat30_path_ptr = NULL;
static const GPathInfo PATH_INFO_30 = {.num_points = 4, .points = (GPoint[]){{37,35},{44,35},{44,38},{38,38}}};
static GPath *s_bat20_path_ptr = NULL;
static const GPathInfo PATH_INFO_20 = {.num_points = 4, .points = (GPoint[]){{39,40},{44,40},{44,43},{40,43}}};
static GPath *s_bat10_path_ptr = NULL;
static const GPathInfo PATH_INFO_10 = {.num_points = 4, .points = (GPoint[]){{41,45},{44,45},{44,48},{41,48}}};
static GPath *s_conn_path_ptr = NULL;
static const GPathInfo PATH_INFO_CONN = {.num_points = 6, .points = (GPoint[]){{0,3},{7,11},{4,14},{4,0},{7,3},{0,11}}};

static void second_update(Layer *layer, GContext *ctx){
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	graphics_context_set_stroke_color(ctx, GColorRed);
	graphics_context_set_stroke_width(ctx, 2);
	double angle_1 = ((double)tick_time->tm_sec * 2.5 - 112.5) * PI / 180;
	double angle_2 = ((double)tick_time->tm_sec * 2.5 - 112.5) * PI / 180;
	double xpos_1 = sin(angle_1) * 68.0 + 72.0;
	double ypos_1 = -cos(angle_1) * 68.0 + 72.0;
	double xpos_2 = sin(angle_2) * 55.0 + 72.0;
	double ypos_2 = -cos(angle_2) * 55.0 + 72.0;
	graphics_draw_line(ctx, GPoint(xpos_1,ypos_1), GPoint(xpos_2,ypos_2));
}

static void battery_update(Layer *layer, GContext *ctx){
	BatteryChargeState bat_stat = battery_state_service_peek();
	graphics_context_set_fill_color(ctx, GColorWhite);
	if(bat_stat.is_charging){gpath_draw_filled(ctx, s_bat_charge_path_ptr);}
	if(bat_stat.charge_percent >= 100){gpath_draw_filled(ctx, s_bat100_path_ptr);}
	if(bat_stat.charge_percent >= 90){gpath_draw_filled(ctx, s_bat90_path_ptr);}
	if(bat_stat.charge_percent >= 80){gpath_draw_filled(ctx, s_bat80_path_ptr);}
	if(bat_stat.charge_percent >= 70){gpath_draw_filled(ctx, s_bat70_path_ptr);}
	if(bat_stat.charge_percent >= 60){gpath_draw_filled(ctx, s_bat60_path_ptr);}
	if(bat_stat.charge_percent >= 50){gpath_draw_filled(ctx, s_bat50_path_ptr);}
	if(bat_stat.charge_percent >= 40){gpath_draw_filled(ctx, s_bat40_path_ptr);}
	if(bat_stat.charge_percent >= 30){gpath_draw_filled(ctx, s_bat30_path_ptr);}
	if(bat_stat.charge_percent >= 20){gpath_draw_filled(ctx, s_bat20_path_ptr);}
	if(bat_stat.charge_percent >= 10){gpath_draw_filled(ctx, s_bat10_path_ptr);}
}

static void inbox_recieved_callback(DictionaryIterator *iterator, void *context){
	static char temp_buffer[] = "--- °C";
	static char hum_buffer[] = "--- %";
	Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_TEMPERATURE);
	Tuple *hum_tuple = dict_find(iterator, MESSAGE_KEY_HUMIDITY);
	
	if(temp_tuple && hum_tuple){
		snprintf(temp_buffer, sizeof("--- °C"), "%d °C", (int)temp_tuple->value->int32);
		snprintf(hum_buffer, sizeof("--- %"), "%d %%", (int)hum_tuple->value->int32);
		text_layer_set_text(s_temp_layer, temp_buffer);
		text_layer_set_text(s_hum_layer, hum_buffer);
	}
}
static void inbox_dropped_callback(AppMessageResult reason, void *context){}
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context){}
static void outbox_sent_callback(DictionaryIterator *iterator, void *context){}

static void weather_update(){
	DictionaryIterator *iterator;
	app_message_outbox_begin(&iterator);
	dict_write_uint8(iterator, 0, 0);
	app_message_outbox_send();
}

static void conn_update(Layer *layer, GContext *ctx){
	graphics_context_set_stroke_color(ctx, GColorWhite);
	graphics_context_set_stroke_width(ctx, 1);
	gpath_draw_outline_open(ctx, s_conn_path_ptr);
}

static void update_by_day(struct tm *tick_time){
	static char date_buffer[] = "01/01";
	strftime(date_buffer, sizeof("01/01"), "%m/%d", tick_time);
	text_layer_set_text(s_date_layer, date_buffer);
	
	static char weekday_buffer[] = "sun";
	switch(tick_time->tm_wday){
		case 0: snprintf(weekday_buffer, sizeof("sun"), "%s","Sun"); break;
		case 1: snprintf(weekday_buffer, sizeof("mon"), "%s","Mon"); break;
		case 2: snprintf(weekday_buffer, sizeof("tue"), "%s","Tue"); break;
		case 3: snprintf(weekday_buffer, sizeof("wed"), "%s","Wed"); break;
		case 4: snprintf(weekday_buffer, sizeof("thu"), "%s","Thu"); break;
		case 5: snprintf(weekday_buffer, sizeof("fri"), "%s","Fri"); break;
		case 6: snprintf(weekday_buffer, sizeof("sat"), "%s","Sat"); break;
	}
	text_layer_set_text(s_weekday_layer, weekday_buffer);
}

static void update_by_minute(struct tm *tick_time){
	static char time_buffer[] = "00:00";
	if(clock_is_24h_style() == true){
		strftime(time_buffer, sizeof("00:00"), "%H:%M", tick_time);
	}else{
		strftime(time_buffer, sizeof("00:00"), "%I:%M", tick_time);
	}
	text_layer_set_text(s_time_layer, time_buffer);
	
	static char steps_buffer[] = "99999 steps";
	snprintf(steps_buffer, sizeof("99999 steps"), "%d steps", (int)health_service_sum_today(HealthMetricStepCount));
	text_layer_set_text(s_step_layer, steps_buffer);
	
	static char cals_buffer[] = "99999 kcals";
	snprintf(cals_buffer, sizeof("99999 kcals"), "%d kcals",
					 (int)health_service_sum_today(HealthMetricActiveKCalories)+(int)health_service_sum_today(HealthMetricRestingKCalories));
	text_layer_set_text(s_cals_layer, cals_buffer);
	
	if(tick_time->tm_min % 30 == 0){
		weather_update();
	}
	if(tick_time->tm_hour == 0){
		update_by_day(tick_time);
	}
}

static void battery_handler(BatteryChargeState charge_stat){
	layer_mark_dirty(s_bat_layer);
}

static void conn_handler(bool connected){
	layer_set_hidden(s_conn_layer, !connected);
	if(!connected){
		vibes_long_pulse();
	}
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
	layer_mark_dirty(s_seconds_layer);
	if(units_changed == MINUTE_UNIT || tick_time->tm_sec == 0){
		update_by_minute(tick_time);
	}
	if(tick_time->tm_hour >= 6 && tick_time->tm_hour < 22){
		if (units_changed == MINUTE_UNIT){
			tick_timer_service_unsubscribe();
			tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
		}
	}else{
		if (units_changed == SECOND_UNIT){
			tick_timer_service_unsubscribe();
			tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
		}
	}
}

static void main_window_load(Window *window){
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_base);
	s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
	s_time_layer = text_layer_create(GRect(22,72,100,26));
	text_layer_set_background_color(s_time_layer, GColorClear);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	text_layer_set_text(s_time_layer, "00:00");
	text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM));
	text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	s_date_layer = text_layer_create(GRect(32,57,80,18));
	text_layer_set_background_color(s_date_layer, GColorClear);
	text_layer_set_text_color(s_date_layer, GColorWhite);
	text_layer_set_text(s_date_layer, "01/01");
	text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
	
	s_weekday_layer = text_layer_create(GRect(32,98,80,18));
	text_layer_set_background_color(s_weekday_layer, GColorClear);
	text_layer_set_text_color(s_weekday_layer, GColorWhite);
	text_layer_set_text(s_weekday_layer, "sun");
	text_layer_set_font(s_weekday_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
	text_layer_set_text_alignment(s_weekday_layer, GTextAlignmentCenter);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weekday_layer));
	
	s_seconds_layer = layer_create(GRect(0,20,144,144));
	layer_set_update_proc(s_seconds_layer, second_update);
	layer_add_child(window_get_root_layer(window), s_seconds_layer);
	
	s_bat_layer = layer_create(GRect(100,24,44,48));
	layer_set_update_proc(s_bat_layer, battery_update);
	layer_add_child(window_get_root_layer(window), s_bat_layer);
	s_bat_charge_path_ptr = gpath_create(&PATH_INFO_CHARGE);
	s_bat100_path_ptr = gpath_create(&PATH_INFO_100);
	s_bat90_path_ptr = gpath_create(&PATH_INFO_90);
	s_bat80_path_ptr = gpath_create(&PATH_INFO_80);
	s_bat70_path_ptr = gpath_create(&PATH_INFO_70);
	s_bat60_path_ptr = gpath_create(&PATH_INFO_60);
	s_bat50_path_ptr = gpath_create(&PATH_INFO_50);
	s_bat40_path_ptr = gpath_create(&PATH_INFO_40);
	s_bat30_path_ptr = gpath_create(&PATH_INFO_30);
	s_bat20_path_ptr = gpath_create(&PATH_INFO_20);
	s_bat10_path_ptr = gpath_create(&PATH_INFO_10);
	
	s_conn_layer = layer_create(GRect(132,5,8,20));
	layer_set_update_proc(s_conn_layer, conn_update);
	layer_add_child(window_get_root_layer(window), s_conn_layer);
	s_conn_path_ptr = gpath_create(&PATH_INFO_CONN);
	layer_mark_dirty(s_conn_layer);
	
	s_step_layer = text_layer_create(GRect(80,146,60,12));
	text_layer_set_background_color(s_step_layer, GColorClear);
	text_layer_set_text_color(s_step_layer, GColorWhite);
	text_layer_set_text(s_step_layer, "99999 steps");
	text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_09));
	text_layer_set_text_alignment(s_step_layer, GTextAlignmentRight);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_step_layer));
	
	s_cals_layer = text_layer_create(GRect(80,156,60,12));
	text_layer_set_background_color(s_cals_layer, GColorClear);
	text_layer_set_text_color(s_cals_layer, GColorWhite);
	text_layer_set_text(s_cals_layer, "99999 kcals");
	text_layer_set_font(s_cals_layer, fonts_get_system_font(FONT_KEY_GOTHIC_09));
	text_layer_set_text_alignment(s_cals_layer, GTextAlignmentRight);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_cals_layer));
	
	s_temp_layer = text_layer_create(GRect(4,146,20,12));
	text_layer_set_background_color(s_temp_layer, GColorClear);
	text_layer_set_text_color(s_temp_layer, GColorWhite);
	text_layer_set_text(s_temp_layer, "--- °C");
	text_layer_set_font(s_temp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_09));
	text_layer_set_text_alignment(s_temp_layer, GTextAlignmentRight);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temp_layer));
	
	s_hum_layer = text_layer_create(GRect(4,156,20,12));
	text_layer_set_background_color(s_hum_layer, GColorClear);
	text_layer_set_text_color(s_hum_layer, GColorWhite);
	text_layer_set_text(s_hum_layer, "--- %");
	text_layer_set_font(s_hum_layer, fonts_get_system_font(FONT_KEY_GOTHIC_09));
	text_layer_set_text_alignment(s_hum_layer, GTextAlignmentRight);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hum_layer));
	
	app_message_register_inbox_received(inbox_recieved_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);
	
	const int inbox_size = 128;
	const int outbox_size = 128;
	app_message_open(inbox_size, outbox_size);
	
	battery_state_service_subscribe(battery_handler);
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	connection_service_subscribe((ConnectionHandlers){.pebble_app_connection_handler = conn_handler});
	
	weather_update();
	conn_handler(connection_service_peek_pebble_app_connection());
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	update_by_day(tick_time);
	update_by_minute(tick_time);
}

static void main_window_unload(Window *window){
	connection_service_unsubscribe();
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	app_message_deregister_callbacks();
	text_layer_destroy(s_hum_layer);
	text_layer_destroy(s_temp_layer);
	text_layer_destroy(s_cals_layer);
	text_layer_destroy(s_step_layer);
	gpath_destroy(s_conn_path_ptr);
	layer_destroy(s_conn_layer);
	gpath_destroy(s_bat10_path_ptr);
	gpath_destroy(s_bat20_path_ptr);
	gpath_destroy(s_bat30_path_ptr);
	gpath_destroy(s_bat40_path_ptr);
	gpath_destroy(s_bat50_path_ptr);
	gpath_destroy(s_bat60_path_ptr);
	gpath_destroy(s_bat70_path_ptr);
	gpath_destroy(s_bat80_path_ptr);
	gpath_destroy(s_bat90_path_ptr);
	gpath_destroy(s_bat100_path_ptr);
	gpath_destroy(s_bat_charge_path_ptr);
	layer_destroy(s_bat_layer);
	layer_destroy(s_seconds_layer);
	text_layer_destroy(s_date_layer);
	text_layer_destroy(s_weekday_layer);
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
}

static void deinit(){
	window_destroy(s_main_window);
}

int main(void){
	init();
	app_event_loop();
	deinit();
}
