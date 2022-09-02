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
  char *display_text;
} GUIState;

static void c_ruby_gui_input_callback(InputEvent* input_event, void* ctx) {
  FURI_LOG_I(TAG, "Input Callback");

  FuriMessageQueue* event_queue = ctx;
  furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

static void c_ruby_gui_render_callback(Canvas* canvas, void* gui_mutex) {
  GUIState *state = (GUIState*)acquire_mutex((ValueMutex*)gui_mutex, 25);
  char *text = state->display_text;

  elements_multiline_text_aligned(
      canvas,
      0,
      0,
      AlignLeft,
      AlignTop,
      text);

  release_mutex((ValueMutex*)gui_mutex, state);
}

static void c_ruby_gui_set_text(mrb_vm *vm, mrb_value v[], int argc) {
  if( argc != 2 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }
  FURI_LOG_I(TAG, "Self is %i", mrbc_integer(v[0]));

  ValueMutex *gui_mutex = v[2].handle;
  furi_check(gui_mutex);
  GUIState *state = (GUIState*)acquire_mutex(gui_mutex, 25);
  char* text = RSTRING_PTR(v[1]);
  if(state->display_text) free((void*)state->display_text);
  state->display_text = strdup(text);
  release_mutex(gui_mutex, state);

  FURI_LOG_I(TAG, "Set text: %s", text);
}

static void c_ruby_gui_update_view(mrb_vm *vm, mrb_value v[], int argc) {
  UNUSED(vm);
  UNUSED(argc);

  if( argc != 1 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }

  ValueMutex *gui_mutex = v[1].handle;
  furi_check(gui_mutex);
  GUIState *state = (GUIState*)acquire_mutex(gui_mutex, 25);
  if ( state == NULL ) {
    FURI_LOG_E(TAG, "Could not get mutex");
    return;
  }
  view_port_update(state->view_port);
  release_mutex(gui_mutex, state);
}

static void c_ruby_gui_input_event(mrb_vm *vm, mrb_value v[], int argc) {
  if( argc != 1 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }

  ValueMutex *gui_mutex = v[1].handle;
  GUIState *state = (GUIState*)acquire_mutex(gui_mutex, 25);
  release_mutex(gui_mutex, state);

  InputEvent event;
  if(furi_message_queue_get(state->event_queue, &event, 25) == FuriStatusOk) {
    if(event.type == InputTypeShort) {
      const char* key_name = input_get_key_name(event.key);
      FURI_LOG_I(TAG, "Key name is %s", key_name);
      mrbc_value str = mrbc_string_new_cstr(vm, key_name);
      SET_RETURN(str);
    } else {
      FURI_LOG_I(TAG, "Didn't know the event type");
      SET_RETURN( mrbc_nil_value() );
    }
  } else {
    SET_RETURN( mrbc_nil_value() );
  }
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

  ViewPort* view_port = view_port_alloc();
  // Open GUI and register view_port
  Gui* gui = furi_record_open(RECORD_GUI);
  gui_add_view_port(gui, view_port, GuiLayerFullscreen);

  GUIState* gui_state = malloc(sizeof(GUIState));
  gui_state->event_queue = event_queue;
  gui_state->view_port = view_port;
  gui_state->gui = gui;

  //Stick the object in a mutex
  ValueMutex *gui_mutex = malloc(sizeof(ValueMutex));
  if (!init_mutex(gui_mutex, gui_state, sizeof(GUIState))) {
    FURI_LOG_E(TAG, "FAILED MUTEX");
    free(gui_state);
    free(gui_mutex);
    return;
  }

  view_port_draw_callback_set(view_port, c_ruby_gui_render_callback, gui_mutex);
  view_port_input_callback_set(view_port, c_ruby_gui_input_callback, event_queue);

  FURI_LOG_I(TAG, "Started GUI");
  mrbc_value rv = {.tt = MRBC_TT_HANDLE};
  rv.handle = gui_mutex;
  SET_RETURN(rv);
}

static void c_ruby_gui_goodbye(mrb_vm *vm, mrb_value v[], int argc) {
  FURI_LOG_I(TAG, "Ending GUI");
  UNUSED(vm);
  UNUSED(argc);

  if( argc != 1 )
  {
    mrbc_raise( vm, MRBC_CLASS(ArgumentError), 0 );
    return;
  }

  ValueMutex *gui_mutex = v[1].handle;
  GUIState *gui_state = (GUIState*)acquire_mutex(gui_mutex, 25);
  view_port_enabled_set(gui_state->view_port, false);
  gui_remove_view_port(gui_state->gui, gui_state->view_port);
  view_port_free(gui_state->view_port);
  furi_message_queue_free(gui_state->event_queue);
  furi_record_close(RECORD_GUI);
  release_mutex(gui_mutex, gui_state);
  delete_mutex(gui_mutex);
  free(gui_state);
}

void make_ruby_gui_class(mrb_vm *vm)
{
  mrb_class *cls = mrbc_define_class(vm, "GUI", mrbc_class_object);
  mrbc_define_method(vm, cls, "start", c_ruby_gui_hello);
  mrbc_define_method(vm, cls, "close", c_ruby_gui_goodbye);
  mrbc_define_method(vm, cls, "set_text", c_ruby_gui_set_text);
  mrbc_define_method(vm, cls, "update_view", c_ruby_gui_update_view);
  mrbc_define_method(vm, cls, "fetch_input", c_ruby_gui_input_event);
}

#undef TAG
