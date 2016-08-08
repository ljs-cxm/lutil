/* trim.c - based on http://lua-users.org/lists/lua-l/2009-12/msg00951.html
   from Sean Conner */

#include <ctype.h>
#include <lua.h>
#include <lauxlib.h>

#define LUTIL_TRIM_VARIABLES                                                   \
    const char *front;                                                         \
    const char *end;                                                           \
    size_t size;                                                               \
                                                                               \
    front = luaL_checklstring(L, 1, &size);                                    \
    end = &front[size - 1];

#define LUTIL_TRIM_LEFT                                                        \
    for (; size && isspace(*front); size--, front++)                           \
        ;

#define LUTIL_TRIM_RIGHT                                                       \
    for (; size && isspace(*end); size--, end--)                               \
        ;

#define LUTIL_TRIM_RETURN                                                      \
    lua_pushlstring(L, front, (size_t)(end - front) + 1);                      \
    return 1;

static int trim(lua_State *L) {
    LUTIL_TRIM_VARIABLES
    LUTIL_TRIM_LEFT
    LUTIL_TRIM_RIGHT
    LUTIL_TRIM_RETURN
}

static int ltrim(lua_State *L) {
    LUTIL_TRIM_VARIABLES
    LUTIL_TRIM_LEFT
    LUTIL_TRIM_RETURN
}

static int rtrim(lua_State *L) {
    LUTIL_TRIM_VARIABLES
    LUTIL_TRIM_RIGHT
    LUTIL_TRIM_RETURN
}

static const struct luaL_Reg lutil_lib[] = {
    {"trim", trim}, {"ltrim", ltrim}, {"rtrim", rtrim}, {NULL, NULL}};

int luaopen_lutil(lua_State *L) {
#if LUA_VERSION_NUM < 502
    luaL_register(L, "lutil", lutil_lib);
#else
    luaL_newlib(L, lutil_lib);
#endif

    return 1;
}

/* vi:set ft=c ts=4 sw=4 et fdm=marker: */
