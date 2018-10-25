
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

static int l_blocknum(lua_State *L)
{
  uint64_t uid = lua_tointeger(L, 1);
  lua_pushinteger(L, myblocks_blocknum(uid));
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
    lua_createtable(L, 10, 0);
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
    lua_pushstring(L, "nbuttons");
    lua_pushinteger(L, info.nbuttons);
    lua_settable(L, -3);
    lua_pushstring(L, "nleds");
    lua_pushinteger(L, info.nleds);
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
  } else {
    lua_pushnil(L);
  }
  return 1;
}

static int l_get_byte(lua_State *L)
{
  int blocknum = lua_tointeger(L, 1);
  size_t offset = lua_tointeger(L, 2);
  lua_pushinteger(L, myblocks_get_byte(blocknum, offset));
  return 1;
}

static int l_set_byte(lua_State *L)
{
  int blocknum = lua_tointeger(L, 1);
  size_t offset = lua_tointeger(L, 2);
  uint8_t data = lua_tointeger(L, 3);
  myblocks_set_byte(blocknum, offset, data);
  return 0;
}

static int l_set_button(lua_State *L)
{
  int blocknum = lua_tointeger(L, 1);
  int num = lua_tointeger(L, 2);
  unsigned color = lua_tointeger(L, 3);
  myblocks_set_button(blocknum, num, color);
  return 0;
}

static int l_set_leds(lua_State *L)
{
  int blocknum = lua_tointeger(L, 1);
  int num = lua_tointeger(L, 2);
  unsigned color = lua_tointeger(L, 3);
  myblocks_set_leds(blocknum, num, color);
  return 0;
}

static int l_send(lua_State *L)
{
  if (!lua_isnumber(L, 1) || !lua_istable(L, 2)) return 0;
  int blocknum = lua_tointeger(L, 1);
  size_t len = lua_rawlen(L, 2);
  if (len == 0) return 0;
  int msg[3] = { 0, 0, 0 };
  for (size_t i = 0; i < 3 && i < len; i++) {
    if (lua_rawgeti(L, 2, i+1) != LUA_TNUMBER) return 0;
    msg[i] = lua_tointeger(L, -1);
    lua_pop(L, 1);
  }
  myblocks_send(blocknum, msg);
  return 0;
}

static int l_receive(lua_State *L)
{
  int blocknum, msg[3];
  myblocks_button_info_t button_info;
  if (myblocks_receive(&blocknum, msg, &button_info)) {
    lua_pushinteger(L, blocknum);
    if (button_info.name) {
      lua_createtable(L, 4, 0);
      lua_pushstring(L, "name");
      lua_pushstring(L, button_info.name);
      lua_settable(L, -3);
      lua_pushstring(L, "num");
      lua_pushinteger(L, button_info.num);
      lua_settable(L, -3);
      lua_pushstring(L, "type");
      lua_pushinteger(L, button_info.type);
      lua_settable(L, -3);
      lua_pushstring(L, "pressed");
      lua_pushboolean(L, button_info.pressed);
      lua_settable(L, -3);
    } else {
      int i = 0;
      lua_createtable(L, 3, 0);
      lua_pushinteger(L, ++i);
      lua_pushinteger(L, msg[0]);
      lua_settable(L, -3);
      lua_pushinteger(L, ++i);
      lua_pushinteger(L, msg[1]);
      lua_settable(L, -3);
      lua_pushinteger(L, ++i);
      lua_pushinteger(L, msg[2]);
      lua_settable(L, -3);
    }
  } else {
    lua_pushnil(L);
    lua_pushnil(L);
  }
  return 2;
}

static const struct luaL_Reg l_myblocks [] = {
  {"msleep", l_msleep},
  {"start", l_start},
  {"stop", l_stop},
  {"process", l_process},
  {"changed", l_changed},
  {"count_blocks", l_count_blocks},
  {"blocknum", l_blocknum},
  {"set_program", l_set_program},
  {"load_program", l_load_program},
  {"save_program", l_save_program},
  {"reset", l_reset},
  {"factory_reset", l_factory_reset},
  {"msg", l_msg},
  {"info", l_info},
  {"get_byte", l_get_byte},
  {"set_byte", l_set_byte},
  {"set_button", l_set_button},
  {"set_leds", l_set_leds},
  {"send", l_send},
  {"receive", l_receive},
  {NULL, NULL}  /* sentinel */
};

int luaopen_myblocks(lua_State *L) {
  luaL_newlib(L, l_myblocks);
  return 1;
}
