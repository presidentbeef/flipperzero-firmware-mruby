#include "mrubyc.h"
#include <furi_hal_resources.h>
#include <gui/gui.h>
#include <input/input.h>
#include <gui/elements.h>

#define TAG "Ruby GUI"

typedef struct GUIState {
  FuriMessageQueue *event_queue;
  ViewPort *view_port;
  Gui *gui;
  mrb_value *rgui;
} GUIState;

static ValueMutex gui_mutex;

static void c_ruby_gui_goodbye(GUIState *state);

static void c_ruby_gui_input_callback(InputEvent* input_event, void* ctx) {
  UNUSED(ctx);

  FURI_LOG_I(TAG, "Input Callback");

  GUIState *state = (GUIState*)acquire_mutex(&gui_mutex, 25);
  release_mutex(&gui_mutex, state);
  if(input_event->key == InputKeyBack) {
    c_ruby_gui_goodbye(state);
  }
  
}

static void c_ruby_gui_render_callback(Canvas* canvas, void* ctx) {
  UNUSED(ctx);
  UNUSED(canvas);

  FURI_LOG_I(TAG, "Render Callback");
  GUIState *state = (GUIState*)acquire_mutex(&gui_mutex, 25);
  release_mutex(&gui_mutex, state);
  mrb_value* gui = state->rgui;
  mrb_value display_text = mrbc_instance_getiv(gui, mrbc_str_to_symid("display_text"));
  char* text = RSTRING_PTR(display_text);
  FURI_LOG_I(TAG, "Display: %d", mrbc_integer(display_text));

  elements_multiline_text_aligned(
      canvas,
      0,
      0,
      AlignLeft,
      AlignTop,
      text);
}

static void c_ruby_gui_set_text(mrb_vm *vm, mrb_value v[], int argc) {
  if( argc != 1 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }
  FURI_LOG_I(TAG, "Self is %i", mrbc_integer(v[0]));

  mrb_value display_text = mrbc_instance_getiv(&v[0], mrbc_str_to_symid("display_text"));
  char* text = RSTRING_PTR(display_text);
  FURI_LOG_I(TAG, "Before: %s", text);
  mrbc_instance_setiv(&v[0], mrbc_str_to_symid("display_text"), &v[1]);

  GUIState *state = (GUIState*)acquire_mutex(&gui_mutex, 25);
  release_mutex(&gui_mutex, state);
  view_port_update(state->view_port);
}

/*
static void invoke_method(mrb_vm *vm, mrb_value v[], int argc, char* method_name) {

  mrbc_class *cls = v[0].instance->cls;
  mrbc_sym method_sym = mrbc_str_to_symid(method_name);

  mrbc_method method;
  if( mrbc_find_method( &method, cls, method_sym ) == NULL ) return;

//  mrbc_decref(&v[argc+1]);
//  mrbc_set_nil(&v[argc+1]);
  mrbc_callinfo *callinfo = mrbc_push_callinfo(vm, method_sym, (v - vm->cur_regs), argc);

  callinfo->own_class = method.cls;
  vm->cur_irep = method.irep;
  vm->inst = vm->cur_irep->inst;
  vm->cur_regs = v;
}
*/

static void c_ruby_gui_hello(mrb_vm *vm, mrb_value v[], int argc) {
  FURI_LOG_I(TAG, "Starting GUI stuff?");

  if( argc != 0 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
 }

  FuriMessageQueue* event_queue = furi_message_queue_alloc(32, sizeof(InputEvent));
  furi_check(event_queue);

  FURI_LOG_I(TAG, "Oi");

  ViewPort* view_port = view_port_alloc();
  // Open GUI and register view_port
  Gui* gui = furi_record_open(RECORD_GUI);
  gui_add_view_port(gui, view_port, GuiLayerFullscreen);

  FURI_LOG_I(TAG, "Hmmm");

  GUIState* gui_state = malloc(sizeof(GUIState));
  FURI_LOG_I(TAG, "1");
  gui_state->event_queue = event_queue;
  FURI_LOG_I(TAG, "2");
  gui_state->view_port = view_port;
  FURI_LOG_I(TAG, "3");
  gui_state->gui = gui;
  FURI_LOG_I(TAG, "4");
  gui_state->rgui = &v[0];
  FURI_LOG_I(TAG, "5");

  FURI_LOG_I(TAG, "Init the mutex!");

  //Stick the object in a mutex
  if (!init_mutex(&gui_mutex, gui_state, sizeof(GUIState))) {
    FURI_LOG_E(TAG, "FAILED MUTEX");
    free(gui_state);
    return;
  }

  FURI_LOG_I(TAG, "Did it...");

  view_port_draw_callback_set(view_port, c_ruby_gui_render_callback, NULL);
  view_port_input_callback_set(view_port, c_ruby_gui_input_callback, NULL);

  /*
  InputEvent event;
  while(furi_message_queue_get(event_queue, &event, FuriWaitForever) == FuriStatusOk) {
        if(event.type == InputTypeShort) {
          const char* key_name = input_get_key_name(event.key);

          mrb_value* rgui = acquire_mutex((ValueMutex*)&gui_mutex, 25);
          FURI_LOG_I(TAG, "rgui is %i", mrbc_integer(v[0]));
          mrbc_value str = mrbc_string_new_cstr(vm, key_name);

          mrbc_instance_setiv(rgui, mrbc_str_to_symid("display_text"), &str);
          invoke_method(vm, v, argc, "handle_input");
          release_mutex((ValueMutex*)&gui_mutex, rgui);

          if(event.key == InputKeyBack) {
            break;
          }
        }

        view_port_update(view_port);
   }
   */

  FURI_LOG_I(TAG, "Started GUI stuff?");
}

static void c_ruby_gui_goodbye(GUIState *gui_state) {
  FURI_LOG_I(TAG, "Ending GUI stuff");

  gui_remove_view_port(gui_state->gui, gui_state->view_port);
  view_port_free(gui_state->view_port);
  furi_message_queue_free(gui_state->event_queue);
  furi_record_close(RECORD_GUI);
  delete_mutex(&gui_mutex);
}

void make_ruby_gui_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "GUI", mrbc_class_object);
  mrbc_define_method(vm, cls, "start", c_ruby_gui_hello);
  mrbc_define_method(vm, cls, "set_text", c_ruby_gui_set_text);
}


#undef TAG
