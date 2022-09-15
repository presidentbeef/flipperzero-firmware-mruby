#include "mrubyc.h"
#include <furi_hal_resources.h>

#define TAG "Ruby GUI"

static void c_ruby_led_on(mrb_vm *vm, mrbc_value v[], int argc) {
  if( argc != 1 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
  }

  mrbc_value opt_hash = GET_ARG(1);

  NotificationMessage notification_led_message_red;
  NotificationMessage notification_led_message_green;
  NotificationMessage notification_led_message_blue;

  int red = 0;
  int green = 0;
  int blue = 0;
  
  mrbc_value red_sym = mrbc_symbol_value(mrbc_str_to_symid("red"));
  mrbc_value red_value = mrbc_hash_get(&opt_hash, &red_sym);

  mrbc_value green_sym = mrbc_symbol_value(mrbc_str_to_symid("green"));
  mrbc_value green_value = mrbc_hash_get(&opt_hash, &green_sym);

  mrbc_value blue_sym = mrbc_symbol_value(mrbc_str_to_symid("blue"));
  mrbc_value blue_value = mrbc_hash_get(&opt_hash, &blue_sym);

  if (mrbc_type(red_value) != MRBC_TT_NIL) {
    red = mrbc_integer(red_value);
  }

  if (mrbc_type(green_value) != MRBC_TT_NIL) {
    green = mrbc_integer(green_value);
  }

  if (mrbc_type(blue_value) != MRBC_TT_NIL) {
    blue = mrbc_integer(blue_value);
  }

  notification_led_message_red.type = NotificationMessageTypeLedRed;
  notification_led_message_red.data.led.value = red;

  notification_led_message_green.type = NotificationMessageTypeLedGreen;
  notification_led_message_green.data.led.value = green;

  notification_led_message_blue.type = NotificationMessageTypeLedBlue;
  notification_led_message_blue.data.led.value = blue;

  const NotificationSequence notification_sequence = {
    &notification_led_message_red,
    &notification_led_message_blue,
    &notification_led_message_green,
    NULL,
  };

  NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
  notification_internal_message_block(notification, &notification_sequence);
  furi_record_close(RECORD_NOTIFICATION);

  SET_NIL_RETURN();
} 

void make_ruby_led_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "LED", mrbc_class_object);
  mrbc_define_method(vm, cls, "on", c_ruby_led_on);
  //mrbc_define_method(vm, cls, "off", c_ruby_led_off);
}


#undef TAG
