#include "mrubyc.h"
#include <notification/notification_messages.h>

#define TAG "Ruby Notify"

static void c_ruby_notify_led(mrb_vm *vm, mrb_value v[], int argc)
{
  if( argc != 2 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
  }
  else
  {
    string_t led_color;
    string_init(led_color);
    string_set_str(led_color, (const char *) GET_STRING_ARG(1));
    int setting = GET_INT_ARG(2);

    NotificationMessage notification_led_message;

    if(!string_cmp(led_color, "red")) {
        notification_led_message.type = NotificationMessageTypeLedRed;
    } else if(!string_cmp(led_color, "green")) {
        notification_led_message.type = NotificationMessageTypeLedGreen;
    } else if(!string_cmp(led_color, "blue")) {
        notification_led_message.type = NotificationMessageTypeLedBlue;
    } else if(!string_cmp(led_color, "backlight")) {
        notification_led_message.type = NotificationMessageTypeLedDisplayBacklight;
    }

    notification_led_message.data.led.value = setting;

    const NotificationSequence notification_sequence = {
        &notification_led_message,
        NULL,
    };

    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_internal_message_block(notification, &notification_sequence);
    furi_record_close(RECORD_NOTIFICATION);
  }
}

static void c_ruby_notify_vibrate_on(mrb_vm *vm, mrb_value v[], int argc)
{
  UNUSED(v);
  if( argc != 0 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
  }
  else
  {
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message_block(notification, &sequence_set_vibro_on);
    furi_record_close(RECORD_NOTIFICATION);
  }
}

static void c_ruby_notify_vibrate_off(mrb_vm *vm, mrb_value v[], int argc)
{
  UNUSED(v);
  if( argc != 0 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
  }
  else
  {
    NotificationApp* notification = furi_record_open(RECORD_NOTIFICATION);
    notification_message_block(notification, &sequence_reset_vibro);
    furi_record_close(RECORD_NOTIFICATION);
  }
}

void make_mruby_notify_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "Notify", mrbc_class_object);
  mrbc_define_method(vm, cls, "led", c_ruby_notify_led);
  mrbc_define_method(vm, cls, "vibrate_start", c_ruby_notify_vibrate_on);
  mrbc_define_method(vm, cls, "vibrate_stop", c_ruby_notify_vibrate_off);
}

#undef TAG
