#include "../include/libRR.h"
#include <queue>
#include "CDL.hpp"
#include <experimental/filesystem>

#ifdef EMSCRIPTEN
// Emscripten doesn't need a javascript parser
#else
#include "../interpreter/duktape/duktape.h"
#include "../interpreter/mjs/mjs.h"
#endif

#ifdef EMSCRIPTEN

void run_in_duktape(string code) {
  // Emscripten version, call out to JS
}

void run_in_mjs(string code) {
  // Emscripten version, call out to JS
}

#else
static duk_ret_t native_print(duk_context *ctx) {
  printf("%s\n", duk_to_string(ctx, 0));
  return 0;  /* no return value (= undefined) */
}

void run_in_duktape(string code) {
  printf("About to run code: %s in DUKTAPE \n", code.c_str());
  duk_context *ctx = duk_create_heap_default();

  duk_push_c_function(ctx, native_print, 1 /*nargs*/);
  duk_put_global_string(ctx, "print");

  duk_eval_string(ctx, code.c_str());
  printf("%s=%d\n", code.c_str(), (int) duk_get_int(ctx, -1));

  duk_eval_string_noresult(ctx, "function abc() { return 'hi';}; print('hello '+abc());");

  duk_destroy_heap(ctx);
}

void *my_dlsym(void *handle, const char *name) {
  if (strcmp(name, "libRR_get_current_lba") == 0) return (void*)libRR_get_current_lba;
  if (strcmp(name, "libRR_get_current_buffer") == 0) return (void*)libRR_get_current_buffer;
  if (strcmp(name, "memset") == 0) return (void*)memset;
  if (strcmp(name, "libRR_memset") == 0) return (void*)libRR_memset;
  if (strcmp(name, "libRR_replace_lba_buffer") == 0) return (void*)libRR_replace_lba_buffer;

  return NULL;
}

void run_in_mjs(string code) {
  struct mjs *mjs = mjs_create();
  mjs_set_ffi_resolver(mjs, my_dlsym);
  mjs_exec(mjs, code.c_str(), NULL);
}
#endif

#define ENGINE_DUKTAPE 0
#define ENGINE_MJS 1
int current_engine = ENGINE_MJS;

string code_prefix = "let lba = ffi('int libRR_get_current_lba()')(); let buffer = ffi('void* libRR_get_current_buffer()')(); let memset = ffi('void libRR_memset(int, int, int)'); let replace_lba = ffi('void libRR_replace_lba_buffer(int)');";

void libRR_run_script(string code) {
  // printf("About to run code: %s \n", (code_prefix+code).c_str());
  if (current_engine == ENGINE_DUKTAPE) {
    run_in_duktape(code_prefix+code);
  } else if (current_engine == ENGINE_MJS) {
    run_in_mjs(code_prefix+code);
  }
}