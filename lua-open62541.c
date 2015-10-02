#include "lua-open62541.h"


static const struct luaL_Reg open62541 [] = {
    {NULL, NULL} /* sentinel */
};

int luaopen_open62541(lua_State *L) {
    /* metatable for scalar types */
    luaL_newmetatable(L, "open62541-data");
    lua_pushcfunction(L, ua_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, ua_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, ua_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, ua_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);

    /* metatable for arrays */
    luaL_newmetatable(L, "open62541-array");
    lua_pushcfunction(L, ua_array_tostring);
    lua_setfield(L, -2, "__tostring");
    /* branched out in the index function */
    // lua_pushcfunction(L, ua_array_append);
    // lua_setfield(L, -2, "append");
    lua_pushcfunction(L, ua_array_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, ua_array_len);
    lua_setfield(L, -2, "__len");
    lua_pop(L, 1);

    /* create the module */
    luaL_newlib(L, open62541);

    /* if no member in the module is found, try to create a type-new function */
    lua_newtable(L);
    lua_pushcfunction(L, ua_new);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
    return 1;
}
