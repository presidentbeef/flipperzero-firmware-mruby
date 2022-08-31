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
  char *display_text;
} GUIState;

static void c_ruby_gui_goodbye(ValueMutex *gui_mutex);

static void c_ruby_gui_input_callback(InputEvent* input_event, void* gui_mutex) {
  FURI_LOG_I(TAG, "Input Callback");

  GUIState *state = (GUIState*)acquire_mutex((ValueMutex*)gui_mutex, 25);
  release_mutex((ValueMutex*)gui_mutex, state);

  if(input_event->key == InputKeyBack) {
    c_ruby_gui_goodbye(gui_mutex);
  }
}

static void c_ruby_gui_render_callback(Canvas* canvas, void* gui_mutex) {
  FURI_LOG_I(TAG, "Render Callback");
  GUIState *state = (GUIState*)acquire_mutex((ValueMutex*)gui_mutex, 25);
  char *text = state->display_text;
  release_mutex((ValueMutex*)gui_mutex, state);
  FURI_LOG_I(TAG, "Render mutexed");
  furi_check(state);
  /*
  mrb_value* gui = state->rgui;
  FURI_LOG_I(TAG, "Render rgui value");
  release_mutex((ValueMutex*)gui_mutex, state);
  FURI_LOG_I(TAG, "Render released mutex");
  mrb_value display_text = mrbc_instance_getiv(gui, mrbc_str_to_symid("display_text"));
  FURI_LOG_I(TAG, "Got display text");
  char* text = RSTRING_PTR(display_text);
  FURI_LOG_I(TAG, "Display: %d", mrbc_integer(display_text));
  */

  elements_multiline_text_aligned(
      canvas,
      0,
      0,
      AlignLeft,
      AlignTop,
      text);
}

static void c_ruby_gui_set_text(mrb_vm *vm, mrb_value v[], int argc) {
  if( argc != 2 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }
  FURI_LOG_I(TAG, "Self is %i", mrbc_integer(v[0]));

  mrb_value display_text = mrbc_instance_getiv(&v[0], mrbc_str_to_symid("display_text"));
  char* text = RSTRING_PTR(display_text);
  FURI_LOG_I(TAG, "Before: %s", text);
  mrbc_instance_setiv(&v[0], mrbc_str_to_symid("display_text"), &v[1]);

  ValueMutex *gui_mutex = v[2].handle;
  furi_check(gui_mutex);
  GUIState *state = (GUIState*)acquire_mutex(gui_mutex, 25);
  state->display_text = text;
  release_mutex(gui_mutex, state);

  FURI_LOG_I(TAG, "Got text");
}

static void c_ruby_gui_update_view(mrb_vm *vm, mrb_value v[], int argc) {
  UNUSED(vm);
  UNUSED(argc);

  if( argc != 1 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }

  FURI_LOG_I(TAG, "Hello update view");
  FURI_LOG_I(TAG, "Type is %d", v[1].tt);
  ValueMutex *gui_mutex = v[1].handle;
  furi_check(gui_mutex);
  FURI_LOG_I(TAG, "Handle...");
  GUIState *state = (GUIState*)acquire_mutex(gui_mutex, 25);
  FURI_LOG_I(TAG, "Got mutex");
  view_port_update(state->view_port);
  FURI_LOG_I(TAG, "Updated view port");
  release_mutex(gui_mutex, state);
  FURI_LOG_I(TAG, "Released mutex from update");
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
  gui_state->event_queue = event_queue;
  gui_state->view_port = view_port;
  gui_state->gui = gui;
  gui_state->rgui = &v[0];

  //Stick the object in a mutex
  ValueMutex *gui_mutex = malloc(sizeof(ValueMutex));
  if (!init_mutex(gui_mutex, gui_state, sizeof(GUIState))) {
    FURI_LOG_E(TAG, "FAILED MUTEX");
    free(gui_state);
    free(gui_mutex);
    return;
  }

  FURI_LOG_I(TAG, "Did it...");

  view_port_draw_callback_set(view_port, c_ruby_gui_render_callback, gui_mutex);
  view_port_input_callback_set(view_port, c_ruby_gui_input_callback, gui_mutex);

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
  mrbc_value rv = {.tt = MRBC_TT_HANDLE};
  rv.handle = gui_mutex;
  SET_RETURN(rv);
}

static void c_ruby_gui_goodbye(ValueMutex *gui_mutex) {
  FURI_LOG_I(TAG, "Ending GUI stuff");

  GUIState *gui_state = (GUIState*)acquire_mutex(gui_mutex, 25);
  gui_remove_view_port(gui_state->gui, gui_state->view_port);
  view_port_free(gui_state->view_port);
  furi_message_queue_free(gui_state->event_queue);
  furi_record_close(RECORD_GUI);
  delete_mutex(gui_mutex);
  free(gui_state);
}

void make_ruby_gui_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "GUI", mrbc_class_object);
  mrbc_define_method(vm, cls, "start", c_ruby_gui_hello);
  mrbc_define_method(vm, cls, "set_text", c_ruby_gui_set_text);
  mrbc_define_method(vm, cls, "update_view", c_ruby_gui_update_view);
}

#undef TAG
