#include "mrubyc.h"
#include <furi_hal_resources.h>
#include <gui/gui.h>
#include <input/input.h>
#include <gui/elements.h>

#define TAG "Ruby GUI"

static void c_ruby_gui_input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}


static void c_ruby_gui_render_callback(Canvas* canvas, void* ctx) {
  UNUSED(ctx);
    elements_multiline_text(
        canvas,
        0,
        0,
        "\e#Never\e# gonna give you up\n\e!Never\e! gonna let you down");
}

static void c_ruby_gui_hello(mrb_vm *vm, mrb_value v[], int argc) {
  UNUSED(vm);
  UNUSED(v);
  UNUSED(argc);
  FuriMessageQueue* event_queue = furi_message_queue_alloc(32, sizeof(InputEvent));
  furi_check(event_queue);

  ViewPort* view_port = view_port_alloc();
  view_port_draw_callback_set(view_port, c_ruby_gui_render_callback, NULL);
  view_port_input_callback_set(view_port, c_ruby_gui_input_callback, event_queue);

  // Open GUI and register view_port
  Gui* gui = furi_record_open(RECORD_GUI);
  gui_add_view_port(gui, view_port, GuiLayerFullscreen);

  InputEvent event;
    while(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
            if(event.key == InputKeyBack) {
                break;
            }
        }

        view_port_update(view_port);
   }

    // remove & free all stuff created by app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    furi_record_close(RECORD_GUI);
}

void make_ruby_gui_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "GUI", mrbc_class_object);
  mrbc_define_method(vm, cls, "hello", c_ruby_gui_hello);
}


#undef TAG
