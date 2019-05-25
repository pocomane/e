
#ifndef WITH_LUA
int _allow_this_empty_compilation_unit;
#else

#include "script.h"
#include "editor.h"


#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <regex.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>


#include "../vendor/lua-5.3.4/src/lua.h"
#include "../vendor/lua-5.3.4/src/lauxlib.h"
#include "../vendor/lua-5.3.4/src/lualib.h"
lua_State *l;


static void addret(lua_State *l, char* str) {
  const char *r = lua_pushfstring(l, "return %s;", str);
  if (luaL_loadbuffer(l, r, strlen(r), "=stdin") == LUA_OK) lua_remove(l, -2);
  else lua_pop(l, 2);
}


int e_lua_message(lua_State* l) {
  if (lua_gettop(l) == 1) {
    const char* x = lua_tostring(l, 1);

    lua_getglobal(l, "ctx");
    e_context* ctx = lua_touserdata(l, lua_gettop(l));

    e_set_status_msg(ctx, "lua console: %s", x);
  }

  return 0;
}


int e_lua_insert(lua_State* l) {
  if (lua_gettop(l) == 1) {
    const char* x = lua_tostring(l, 1);

    lua_getglobal(l, "ctx");
    e_context* ctx = lua_touserdata(l, lua_gettop(l));

    for (int i = 0; i < strlen(x); i++) e_insert_char(ctx, x[i]);
  }

  return 0;
}


int e_lua_insertn(lua_State* l) {
  if (lua_gettop(l) == 1) {
    const char* x = lua_tostring(l, 1);

    lua_getglobal(l, "ctx");
    e_context* ctx = lua_touserdata(l, lua_gettop(l));

    for (int i = 0; i < strlen(x); i++) e_insert_char(ctx, x[i]);
    e_insert_newline(ctx);
  }

  return 0;
}


int e_lua_delete(lua_State* l) {
  if (lua_gettop(l) == 1) {
    int x = lua_tonumber(l, 1);

    lua_getglobal(l, "ctx");
    e_context* ctx = lua_touserdata(l, lua_gettop(l));

    for (int i = 0; i < x; i++) e_del_char(ctx);
  }

  return 0;
}


int e_lua_move(lua_State* l) {
  if (lua_gettop(l) == 2) {
    int x = lua_tonumber(l, 2);
    int y = lua_tonumber(l, 1);

    lua_getglobal(l, "ctx");
    e_context* ctx = lua_touserdata(l, lua_gettop(l));

    if (y < 0 || y >= ctx->nrows) {
      e_set_status_msg(ctx, "LUA ERROR: tried to set the cursor to line %d (which is invalid)", y);
      return 0;
    }

    int rowl = (ctx->cy >= ctx->nrows) ? 0 : ctx->row[ctx->cy].size;
    if (x < 0 || x >= rowl) {
      e_set_status_msg(ctx, "LUA ERROR: tried to set the cursor to column %d (which is invalid)", x);
      return 0;
    }

    ctx->cx = x;
    ctx->cy = y;
  }

  return 0;
}


int e_lua_get_coords(lua_State* l) {
  lua_getglobal(l, "ctx");
  e_context* ctx = lua_touserdata(l, lua_gettop(l));

  lua_pushnumber(l, ctx->cx);
  lua_pushnumber(l, ctx->cy);

  return 2;
}


int e_lua_get_bounding(lua_State* l) {
  lua_getglobal(l, "ctx");
  e_context* ctx = lua_touserdata(l, lua_gettop(l));

  lua_pushnumber(l, ctx->cols);
  lua_pushnumber(l, ctx->rows);

  return 2;
}


int e_lua_get_text(lua_State* l) {
  lua_getglobal(l, "ctx");
  e_context* ctx = lua_touserdata(l, lua_gettop(l));
  int len;

  char* str = e_rows_to_str(ctx, &len);
  lua_pushstring(l, str);

  free(str);

  return 1;
}


int e_lua_set_tab(lua_State* l) {
  if (lua_gettop(l) == 1) {
    int x = lua_tonumber(l, 1);

    lua_getglobal(l, "ctx");
    e_context* ctx = lua_touserdata(l, lua_gettop(l));

    ctx->tab_width = x;
  }

  return 0;
}


int e_lua_get_tab(lua_State* l) {
  lua_getglobal(l, "ctx");
  e_context* ctx = lua_touserdata(l, lua_gettop(l));
  lua_pushnumber(l, ctx->tab_width);

  return 1;
}


int e_lua_get_filename(lua_State* l) {
  lua_getglobal(l, "ctx");
  e_context* ctx = lua_touserdata(l, lua_gettop(l));

  lua_pushstring(l, ctx->filename);

  return 1;
}


int e_lua_open(lua_State* l) {
  if (lua_gettop(l) == 1) {
    const char* name = lua_tostring(l, 1);

    lua_getglobal(l, "ctx");
    e_context* ctx = lua_touserdata(l, lua_gettop(l));

    int i;
    for (i = 0; i < ctx->nrows; i++) {
      e_free_row(&ctx->row[i]);
    }
    free(ctx->row);
    free(ctx->filename);
    ctx->row = NULL;
    ctx->filename = NULL;
    ctx->nrows = 0;
    ctx->cx = 0;
    ctx->cy = 0;

    e_open(ctx, name);

    lua_pushlightuserdata(l, ctx);
    lua_setglobal(l, "ctx");
  }

  return 0;
}


int e_lua_prompt(lua_State* l) {
  if (lua_gettop(l) == 1) {
    const char* prompt = lua_tostring(l, 1);

    lua_getglobal(l, "ctx");
    e_context* ctx = lua_touserdata(l, lua_gettop(l));

    char* c = e_prompt(ctx, prompt, NULL);

    lua_pushstring(l, c);
  }

  return 1;
}


#define e_lua_move(dir) {\
  if (lua_gettop(l) == 1) {\
    char x = lua_tostring(l, 1)[0];\
    lua_getglobal(l, "ctx");\
    e_context* ctx = lua_touserdata(l, lua_gettop(l));\
    ctx->dir = x;\
  }\
  return 0;\
}


int e_lua_set_left(lua_State* l) {
  e_lua_move(left);
}


int e_lua_set_right(lua_State* l) {
  e_lua_move(right);
}


int e_lua_set_up(lua_State* l) {
  e_lua_move(up);
}


int e_lua_set_down(lua_State* l) {
  e_lua_move(down);
}


#undef e_lua_move


#define e_lua_get_move(dir) {\
  lua_getglobal(l, "ctx");\
  e_context* ctx = lua_touserdata(l, lua_gettop(l));\
  char x[2];\
  x[0] = ctx->dir;\
  x[1] = '\0';\
  lua_pushstring(l, x);\
  return 1;\
}


int e_lua_get_left(lua_State* l) {
  e_lua_get_move(left);
}


int e_lua_get_right(lua_State* l) {
  e_lua_get_move(right);
}


int e_lua_get_up(lua_State* l) {
  e_lua_get_move(up);
}


int e_lua_get_down(lua_State* l) {
  e_lua_get_move(down);
}


int e_lua_add_syntax(lua_State* l) {

  syntax** stx;
  if (lua_gettop(l) == 1) {
    const char* syntax = lua_tostring(l, 1);
    char* dat = strdup(syntax);

    lua_getglobal(l, "ctx");
    e_context* ctx = lua_touserdata(l, lua_gettop(l));

    stx = syntax_init(ctx->stxes, &ctx->stxn, (char*)"lua_stx", dat);
    free(dat);
    if (!stx) {
      luaL_error(l, "Error while parsing syntax."); // Never returns
      return 0;
    }
    ctx->stxes = stx;
  }
  return 0;
}


#undef e_lua_get_move


void e_initialize_lua() {
  l = luaL_newstate();
  luaL_openlibs(l);

  lua_pushcfunction(l, e_lua_message);
  lua_setglobal(l, "message");
  lua_pushcfunction(l, e_lua_insert);
  lua_setglobal(l, "insert");
  lua_pushcfunction(l, e_lua_insertn);
  lua_setglobal(l, "insertn");
  lua_pushcfunction(l, e_lua_delete);
  lua_setglobal(l, "delete");
  lua_pushcfunction(l, e_lua_move);
  lua_setglobal(l, "move");
  lua_pushcfunction(l, e_lua_get_coords);
  lua_setglobal(l, "get_coords");
  lua_pushcfunction(l, e_lua_get_bounding);
  lua_setglobal(l, "get_bounding_rect");
  lua_pushcfunction(l, e_lua_get_text);
  lua_setglobal(l, "get_text");
  lua_pushcfunction(l, e_lua_get_tab);
  lua_setglobal(l, "get_tab");
  lua_pushcfunction(l, e_lua_set_tab);
  lua_setglobal(l, "set_tab");
  lua_pushcfunction(l, e_lua_get_left);
  lua_setglobal(l, "get_left");
  lua_pushcfunction(l, e_lua_set_left);
  lua_setglobal(l, "set_left");
  lua_pushcfunction(l, e_lua_get_right);
  lua_setglobal(l, "get_right");
  lua_pushcfunction(l, e_lua_set_right);
  lua_setglobal(l, "set_right");
  lua_pushcfunction(l, e_lua_get_up);
  lua_setglobal(l, "get_up");
  lua_pushcfunction(l, e_lua_set_up);
  lua_setglobal(l, "set_up");
  lua_pushcfunction(l, e_lua_get_down);
  lua_setglobal(l, "get_down");
  lua_pushcfunction(l, e_lua_set_down);
  lua_setglobal(l, "set_down");
  lua_pushcfunction(l, e_lua_get_filename);
  lua_setglobal(l, "get_filename");
  lua_pushcfunction(l, e_lua_open);
  lua_setglobal(l, "open");
  lua_pushcfunction(l, e_lua_prompt);
  lua_setglobal(l, "prompt");
  lua_newtable(l);
  lua_setglobal(l, "keys");
  lua_newtable(l);
  lua_setglobal(l, "meta_commands");
  lua_pushcfunction(l, e_lua_add_syntax);
  lua_setglobal(l, "add_syntax");
}

char* e_script_eval(e_context* ctx, char* str) {
  char* ret;
  int size;
  if (!l) e_initialize_lua();

  lua_pushlightuserdata(l, ctx);
  lua_setglobal(l, "ctx");

  luaL_loadbuffer(l, str, strlen(str), "=stdin");
  addret(l, str);

  if (lua_pcall(l, 0, 1, 0)) {
    size = snprintf(NULL, 0, "lua can't execute expression: %s.", lua_tostring(l, -1))+1;
    ret = malloc(size*sizeof(char));
    snprintf(ret, size, "lua can't execute expression: %s.", lua_tostring(l, -1));
    lua_pop(l, 1);
  } else {
    if (lua_isnil(l, -1)) {
      return NULL;
    }

    size = snprintf(NULL, 0, "%s", lua_tostring(l, -1))+1;
    ret = malloc(size*sizeof(char));
    snprintf(ret, size, "%s", lua_tostring(l, -1));
    lua_pop(l, lua_gettop(l));
  }

  ret[size-1] = '\0';
  return ret;
}


int e_script_run_file(e_context* ctx, const char* file) {
  // TODO : clean-up
  char* ret = malloc(80*sizeof(char));
  int size = 0;
  if (!l) e_initialize_lua();

  FILE* t = fopen(file, "r");
  if (!t) {
    return E_SCRIPT_NOT_FOUND;
  }
  fclose(t);

  lua_pushlightuserdata(l, ctx);
  lua_setglobal(l, "ctx");

  int err = 0;
  if (luaL_dofile(l, file)) {
    size = 1 + snprintf(NULL, 0, "%s", lua_tostring(l, -1));
    ret = malloc(size*sizeof(char));
    snprintf(ret, size, "%s", lua_tostring(l, -1));
    err = 1;
  } else {
    size = 1;
    ret = malloc(size*sizeof(char));
  }
  lua_pop(l, lua_gettop(l));

  ret[size-1] = '\0';
  if (err) {
    e_set_status_msg(ctx, "Error while executing file: %s.", ret);
  }
  free(ret);

  if (err) return E_SCRIPT_ERROR_WHILE_RUNNING;
  return 0;
}


int e_lua_get_field(const char* key) {
  lua_pushstring(l, key);
  lua_gettable(l, -2);

  if (lua_isnil(l, -1)) return 1;

  return 0;
}


int e_script_meta_command(e_context* ctx, const char* cmd) {
  if (!l) return 1;

  lua_pushlightuserdata(l, ctx);
  lua_setglobal(l, "ctx");

  lua_getglobal(l, "meta_commands");
  if (e_lua_get_field(cmd)) return 1;

  int res = lua_pcall(l, 0, 1, 0);

  if (res == LUA_ERRRUN) {
    e_set_status_msg(ctx, "LUA ERROR: %s", lua_tostring(l, -1));
    lua_pop(l, lua_gettop(l));
  }
  return 0;
}


e_context* e_script_expression_prompt(e_context* ctx) {
  e_context* new = e_history_forward(ctx);
  char* lua_exp = e_prompt(new, "Type Lua expression: %s", NULL);
  if (!lua_exp) return new;
  char* evald = e_script_eval(new, lua_exp);
  if (evald) e_set_status_msg(new, "%s", evald);
  free(lua_exp);
  free(evald);
  return new;
}


int e_script_key(e_context* ctx, int key) {
  if (!l) return 1;
  char x[2];
  x[0] = (char) key;
  x[1] = '\0';

  lua_pushlightuserdata(l, ctx);
  lua_setglobal(l, "ctx");

  lua_getglobal(l, "keys");
  if (e_lua_get_field(x)) return 1;

  return lua_pcall(l, 0, 1, 0);
}


void e_script_free() {
  if (l) lua_close(l);
}


#endif
