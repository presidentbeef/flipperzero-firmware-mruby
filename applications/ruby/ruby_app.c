#include <stdio.h>
#include <stdlib.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include "mrubyc.h"

#define MEMORY_SIZE (1024*30)
static uint8_t memory_pool[MEMORY_SIZE];

#define TAG "Ruby"

int32_t ruby_app(void* p)
{
  UNUSED(p);
  FURI_LOG_I(TAG, "Picking Ruby File");

  string_t file_path;
  string_init(file_path);
  string_set_str(file_path, "/any");

  DialogsApp* dialogs = furi_record_open("dialogs");
  bool res = dialog_file_browser_show(
      dialogs,
      file_path,
      file_path,
      "mrb",
      true,
      &I_ruby_10px,
      false);

  furi_record_close("dialogs");
  if(!res) {
    FURI_LOG_E(TAG, "No file selected");
    string_clear(file_path);
    return 0;
  }

  FURI_LOG_I(TAG, "Loading Ruby bytecode");
  Storage* storage = furi_record_open("storage");
  File* file = storage_file_alloc(storage);

  if(!storage_file_open(file, string_get_cstr(file_path), FSAM_READ, FSOM_OPEN_EXISTING)) {
    FURI_LOG_E(TAG, "Unable to open file");
    storage_file_free(file);
    furi_record_close("storage");
    return 0;
  };

  uint64_t file_size = storage_file_size(file);
  uint8_t *ruby_test = malloc(file_size);

  storage_file_read(file, ruby_test, file_size);

  FURI_LOG_I(TAG, "Starting Ruby!!!");
  mrbc_init(memory_pool, MEMORY_SIZE);

  if( mrbc_create_task(ruby_test, 0) != NULL ){
    FURI_LOG_I(TAG, "Running Ruby!!!");
    mrbc_run();
  }
  else {
    FURI_LOG_I(TAG, "Failed to run Ruby :(");
  }

  FURI_LOG_I(TAG, "Ending Ruby!!!");
  storage_file_free(file);
  string_clear(file_path);
  furi_record_close("storage");
  free(ruby_test);
  FURI_LOG_I(TAG, "Done.");

  return 0;
}