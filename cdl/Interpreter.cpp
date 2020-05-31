#include "../include/libRR.h"
#include <queue>
#include "CDL.hpp"
#include <filesystem>
#include "../interpreter/duktape/duktape.h"
#include "../interpreter/mjs/mjs.h"

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

void run_in_mjs(string code) {
  struct mjs *mjs = mjs_create();
  // mjs_set_ffi_resolver(mjs, my_dlsym);
  mjs_exec(mjs, "print('test')", NULL);
}

#define ENGINE_DUKTAPE 0
#define ENGINE_MJS 1
int current_engine = ENGINE_MJS;

void libRR_run_script(string code) {
  printf("About to run code: %s \n", code.c_str());
  if (current_engine == ENGINE_DUKTAPE) {
    run_in_duktape(code);
  } else if (current_engine == ENGINE_MJS) {
    run_in_mjs(code);
  }
}