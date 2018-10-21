
// Lua interface -------------------------------------------------------------

#include <unistd.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "myblocks.h"

// Something like this should really be in the standard Lua libraries, but it
// isn't. This variant sleep for a given number of millisecs.

static int l_msleep(lua_State *L)
{
  long msecs = lua_tointeger(L, -1);
  usleep(1000*msecs);
  return 0;
}

static int l_start(lua_State *L)
{
  juce_init();
  return 0;
}

static int l_stop(lua_State *L)
{
  juce_fini();
  return 0;
}

static int l_process(lua_State *L)
{
  lua_pushboolean(L, juce_process_events());
  return 1;
}

static int l_changed(lua_State *L)
{
  lua_pushboolean(L, myblocks_changed());
  return 1;
}

static int l_count_blocks(lua_State *L)
{
  lua_pushinteger(L, myblocks_count_blocks());
  return 1;
}

static int l_set_program(lua_State *L)
{
  int blocknum = lua_tointeger(L, 1);
  const char *code = lua_tostring(L, 2);
  lua_pushboolean(L, myblocks_set_program(blocknum, code));
  return 1;
}

static int l_load_program(lua_State *L)
{
  int blocknum = lua_tointeger(L, 1);
  const char *fname = lua_tostring(L, 2);
  lua_pushboolean(L, myblocks_load_program(blocknum, fname));
  return 1;
}

static int l_msg(lua_State *L)
{
  lua_pushstring(L, myblocks_msg());
  return 1;
}

static int l_save_program(lua_State *L)
{
  int blocknum = lua_tointeger(L, -1);
  myblocks_save_program(blocknum);
  return 0;
}

static int l_reset(lua_State *L)
{
  int blocknum = lua_tointeger(L, -1);
  myblocks_reset(blocknum);
  return 0;
}

static int l_factory_reset(lua_State *L)
{
  int blocknum = lua_tointeger(L, -1);
  myblocks_factory_reset(blocknum);
  return 0;
}

static int l_info(lua_State *L)
{
  int blocknum = lua_tointeger(L, -1);
  myblocks_info_t info;
  if (myblocks_info(blocknum, &info)) {
    lua_createtable(L, 7, 0);
    lua_pushstring(L, "uid");
    lua_pushinteger(L, info.uid);
    lua_settable(L, -3);
    lua_pushstring(L, "type");
    lua_pushinteger(L, info.type);
    lua_settable(L, -3);
    lua_pushstring(L, "is_master");
    lua_pushboolean(L, info.is_master);
    lua_settable(L, -3);
    lua_pushstring(L, "is_charging");
    lua_pushboolean(L, info.is_charging);
    lua_settable(L, -3);
    lua_pushstring(L, "battery_level");
    lua_pushnumber(L, info.battery_level);
    lua_settable(L, -3);
    lua_pushstring(L, "descr");
    lua_pushstring(L, info.descr);
    lua_settable(L, -3);
    lua_pushstring(L, "type_descr");
    lua_pushstring(L, info.type_descr);
    lua_settable(L, -3);
    lua_pushstring(L, "serial");
    lua_pushstring(L, info.serial);
    lua_settable(L, -3);
    lua_pushstring(L, "version");
    lua_pushstring(L, info.version);
    lua_settable(L, -3);
    if (info.code) {
      lua_pushstring(L, "code");
      lua_pushstring(L, info.code);
      lua_settable(L, -3);
    }
  } else {
    lua_pushnil(L);
  }
  return 1;
}

static const struct luaL_Reg l_myblocks [] = {
  {"msleep", l_msleep},
  {"start", l_start},
  {"stop", l_stop},
  {"process", l_process},
  {"changed", l_changed},
  {"count_blocks", l_count_blocks},
  {"set_program", l_set_program},
  {"load_program", l_load_program},
  {"save_program", l_save_program},
  {"reset", l_reset},
  {"factory_reset", l_factory_reset},
  {"msg", l_msg},
  {"info", l_info},
  {NULL, NULL}  /* sentinel */
};

int luaopen_myblocks(lua_State *L) {
  luaL_newlib(L, l_myblocks);
  return 1;
}
