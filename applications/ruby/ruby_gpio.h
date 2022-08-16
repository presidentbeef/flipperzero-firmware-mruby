#include "mrubyc.h"
#include <furi_hal_resources.h>

#define TAG "Ruby GPIO"

static const GpioPin* index_to_pin(int index)
{
  switch(index) {
    case 2 :
      return &gpio_ext_pa7;
    case 3 :
      return &gpio_ext_pa6;
  }

  return NULL;
}

static void c_gpio_set_output(mrb_vm *vm, mrb_value v[], int argc)
{
  if( argc != 1 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
  }
  else
  {
    const GpioPin* pin = index_to_pin(mrbc_integer(v[0]));
    furi_hal_gpio_write(pin, false);
    furi_hal_gpio_init_simple(pin, GpioModeOutputPushPull);
  }
}

static void c_gpio_set_input(mrb_vm *vm, mrb_value v[], int argc)
{
  if( argc != 1 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
  }
  else
  {
    const GpioPin* pin = index_to_pin(mrbc_integer(v[0]));
    furi_hal_gpio_init_simple(pin, GpioModeInput);
  }
}

void make_gpio_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "GPIO", mrbc_class_object);
  mrbc_define_method(vm, cls, "hello", c_gpio_hello);
  mrbc_define_method(vm, cls, "set_input", c_gpio_set_input);
  mrbc_define_method(vm, cls, "set_output", c_gpio_set_output);
}
