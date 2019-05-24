
typedef enum {
  E_SCRIPT_OK = 0,
  E_SCRIPT_ERROR_WHILE_RUNNING = -1,
  E_SCRIPT_NOT_FOUND = -2,
} e_script_error;

#ifndef WITH_LUA

#define e_script_eval(A, B) (NULL)
#define e_script_run_file(A, B) (-1)
#define e_script_meta_command(A, B) (-1)
#define e_script_key(A, B) (-1)
#define e_script_free()
#define e_script_expression_prompt(CTX) (e_set_status_msg(CTX, "e wasn't compiled with Lua support."), CTX)

#else

typedef struct e_context e_context;

char* e_script_eval(e_context*, char*);
e_script_error e_script_run_file(e_context*, const char*);
int   e_script_meta_command(e_context*, const char*);
int   e_script_key(e_context*, int);
void  e_script_free();
e_context* e_script_expression_prompt(e_context* ctx);

#endif
