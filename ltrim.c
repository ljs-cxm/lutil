/* trim.c - based on http://lua-users.org/lists/lua-l/2009-12/msg00951.html
   from Sean Conner */

#include "ltrim.h"

#define LUTIL_TRIM_VARIABLES                                                   \
  const char *front;                                                           \
  const char *end;                                                             \
  size_t size;                                                                 \
                                                                               \
  front = luaL_checklstring(L, 1, &size);                                      \
  end = &front[size - 1];

#define LUTIL_TRIM_LEFT                                                        \
  for (; size && isspace(*front); size--, front++)                             \
    ;

#define LUTIL_TRIM_RIGHT                                                       \
  for (; size && isspace(*end); size--, end--)                                 \
    ;

#define LUTIL_TRIM_RETURN                                                      \
  lua_pushlstring(L, front, (size_t)(end - front) + 1);                        \
  return 1;

int lutil_trim(lua_State *L) {
  LUTIL_TRIM_VARIABLES
  LUTIL_TRIM_LEFT
  LUTIL_TRIM_RIGHT
  LUTIL_TRIM_RETURN
}

int lutil_ltrim(lua_State *L) {
  LUTIL_TRIM_VARIABLES
  LUTIL_TRIM_LEFT
  LUTIL_TRIM_RETURN
}

int lutil_rtrim(lua_State *L) {
  LUTIL_TRIM_VARIABLES
  LUTIL_TRIM_RIGHT
  LUTIL_TRIM_RETURN
}

/* vi:set ft=c ts=2 sw=2 et fdm=marker: */
