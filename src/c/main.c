#include <pebble.h>

#define KEY_EARTHQUAKE 0

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_instruction_layer;
bool emergency = 0;

static void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  
  if(emergency){
  if(tick_time->tm_sec % 2 == 0){
      text_layer_set_background_color(s_time_layer,GColorRed);
      light_enable(true);
      vibes_short_pulse();
      }else{
      text_layer_set_background_color(s_time_layer,GColorBlack);
      text_layer_set_text_color(s_instruction_layer, GColorWhite);
      light_enable(false);
      vibes_long_pulse();
  }
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {


  // Store incoming information
  //static char earthquake_buffer[124];

  // Read tuples for data
  Tuple *earthquake_tuple = dict_find(iterator, KEY_EARTHQUAKE);

  // If all data is available, use it
  if(earthquake_tuple) {
    
    if(earthquake_tuple->value->int32 == 11){
      emergency = 1;
      
    }
    if(earthquake_tuple->value->int32 == 12){
      text_layer_set_background_color(s_time_layer,GColorBlack);
      emergency = 0;
    }
    //snprintf(earthquake_buffer, sizeof(earthquake_buffer), "%s", earthquake_tuple->value->cstring);
    
    //text_layer_set_text(s_time_layer, earthquake_buffer);
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

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_background_color(s_time_layer,GColorBlack);
  text_layer_set_text_color(s_instruction_layer, GColorBlack);
  emergency = 0;
  light_enable(false);
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_background_color(s_time_layer,GColorBlack);
  text_layer_set_text_color(s_instruction_layer, GColorBlack);
  emergency = 0;
  light_enable(false);
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_background_color(s_time_layer,GColorBlack);
  text_layer_set_text_color(s_instruction_layer, GColorBlack);
  emergency = 0;
  light_enable(false);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, 0, bounds.size.w, bounds.size.h));

  s_instruction_layer = text_layer_create(
      GRect(0, 64, bounds.size.w, bounds.size.h));
  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  text_layer_set_background_color(s_instruction_layer, GColorClear);
  text_layer_set_text_color(s_instruction_layer, GColorBlack);
  text_layer_set_text(s_instruction_layer, "Press to Mark Safe >");
  text_layer_set_font(s_instruction_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_instruction_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_instruction_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_instruction_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window, click_config_provider);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}