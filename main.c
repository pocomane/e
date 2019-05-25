#include <signal.h>
#include <execinfo.h>
#include "src/editor.h"
#include "src/script.h"
#ifdef __unix__
#ifdef gnome
#include <gtk/gtk.h>
#endif
#endif

#include "defaultsyntax.h"


e_context* GLOB = NULL;


// This does not free.
// After a segfault we shouldn't be messing with memory
void handler(int sig) {
  disable_raw_mode(GLOB);
  void* array[10];
  size_t size;

  size = backtrace(array, 10);

  fputs("e has crashed. If you want to file a bug report, please attach the following:\n", stderr);
  fprintf(stderr, "Signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}


void exitf() {
  disable_raw_mode(GLOB);
  e_context_free(GLOB);
  if (GLOB && GLOB->stxes) {
    syntaxes_free((syntax**)GLOB->stxes);
    GLOB->stxn = 0;
  }
  e_script_free();
}

int main(int argc, char** argv) {
#ifdef __unix__
#ifdef gnome
  gtk_init(&argc, &argv);
#endif
#endif

  GLOB = e_setup();
  GLOB->stxes = syntax_init(GLOB->stxes, &GLOB->stxn, (char*)"embedded_stx", DEFAULT_STX);
  if (!GLOB->stxes) {
    e_set_status_msg(GLOB, "Failed to initialize e: couldn’t read syntax files.");
    return 1;
  }

  signal(SIGSEGV, handler);
  signal(SIGABRT, handler);
  atexit(exitf);

  e_set_highlighting(GLOB, (syntax**)GLOB->stxes);

  int screrr = e_script_run_file(GLOB, (char*) CFGDIR "/rc.lua");

  if (argc > 1) {
    e_open(GLOB, argv[1]);
  }

  if (!GLOB->nrows) {
    e_append_row(GLOB, (char*) "", 0);
    GLOB->dirty = 0;
  }

  if (screrr == E_SCRIPT_OK || screrr == E_SCRIPT_NOT_FOUND){
    e_set_status_msg(GLOB, "Press <Ctrl-p> to insert command (e.g. q to quit, s to save)");
  }

  while(1) {
    e_clear_screen(GLOB);
    GLOB = e_process_key(GLOB);

    // for reasons I'm not quite certain of this fixes the screen flicker.
    // TODO: investigate better technique
    msleep(20);
  }
  return 0;
}
