#include <stdio.h>
#include <string.h>
#include "lua5.2/lua.h"
#include "lua5.2/lauxlib.h"
#include "lua5.2/lualib.h"

#include "open62541.h"

/*****************/
/* Builtin Types */
/*****************/

/* String */
static int string_new(lua_State *L) {
    int top = lua_gettop(L);
    if(top > 1)
        return luaL_error(L, "Too many arguments");
    if(top == 0) {
        UA_String *s = lua_newuserdata(L, sizeof(UA_String));
        UA_String_init(s);
    } else {
        if(!lua_isstring(L, 1))
            return luaL_error(L, "Argument is not a string");
        size_t length;
        const char *array = lua_tolstring(L, -1, &length);
        UA_String *s = lua_newuserdata(L, sizeof(UA_String));
        *s = UA_String_fromChars(array);
    }
    luaL_getmetatable(L, "open62541-string");
    lua_setmetatable(L, -2);
    return 1;
}

static int string_tostring(lua_State *L) {
    UA_String *s = luaL_checkudata (L, -1, "open62541-string");
    if(s->length <= 0)
        lua_pushlstring(L, NULL, 0);
    else
        lua_pushlstring(L, s->data, s->length);
    return 1;
}

static int string_gc(lua_State *L) {
    UA_String *s = luaL_checkudata (L, -1, "open62541-string");
    UA_String_deleteMembers(s);
    return 0;
}

/* NodeId */
static int nodeid_new(lua_State *L) {
    if(lua_gettop(L) != 2)
        return luaL_error(L, "nodeid takes two arguments");
    if(!lua_isnumber(L, 1))
        return luaL_error(L, "The first argument is no integer namespace id");
    if(!lua_isnumber(L, 2) && !lua_isstring(L, 2))
        return luaL_error(L, "Currently, only numeric and string nodeids are implemented");

    UA_NodeId *id = lua_newuserdata(L, sizeof(UA_NodeId));
    UA_NodeId_init(id);
    luaL_getmetatable(L, "open62541-nodeid");
    lua_setmetatable(L, -2);

    lua_Integer nsindex = lua_tointeger(L, 1);
    id->namespaceIndex = nsindex;
    if(lua_isnumber(L, 2)) {
        id->identifierType = UA_NODEIDTYPE_NUMERIC;
        id->identifier.numeric = lua_tointeger(L, 2);
    } else if(lua_isstring(L, 2)) {
        id->identifierType = UA_NODEIDTYPE_STRING;
        size_t length;
        const char *array = lua_tolstring(L, 2, &length);
        id->identifier.string = UA_String_fromChars(array);
    } else
        return luaL_error(L, "Unknown nodeid type");
    
    return 1;
}

static int nodeid_tostring(lua_State *L) {
    UA_NodeId *id = luaL_checkudata (L, -1, "open62541-nodeid");
    char out[512];
    int size;
    if(id->identifierType == UA_NODEIDTYPE_NUMERIC)
        size = snprintf(out, 512, "nodeid(ns=%i,i=%i)", id->namespaceIndex, id->identifier.numeric);
    else if(id->identifierType == UA_NODEIDTYPE_STRING)
        size = snprintf(out, 512, "nodeid(ns=%i,s=%.*s)", id->namespaceIndex,
                        id->identifier.string.length, id->identifier.string.data);
    else
        return luaL_error(L, "Unknown nodeid type");
        
    lua_pushlstring(L, out, size);
    return 1;
}

static int nodeid_gc(lua_State *L) {
    UA_NodeId *id = luaL_checkudata (L, -1, "open62541-nodeid");
    UA_NodeId_deleteMembers(id);
    return 0;
}

/***********************/
/* Populate the Module */
/***********************/

static const struct luaL_Reg open62541 [] = {
    {"string", string_new},
    {"nodeid", nodeid_new},
    {NULL, NULL} /* sentinel */
};

static void register_builtintypes(lua_State *L) {
    /* String */
    luaL_newmetatable(L, "open62541-string");
    lua_pushcfunction(L, string_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, string_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    /* NodeId */
    luaL_newmetatable(L, "open62541-nodeid");
    lua_pushcfunction(L, nodeid_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, nodeid_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);
}

int luaopen_open62541(lua_State *L) {
    register_builtintypes(L);
    luaL_newlib(L, open62541);
    return 1;
}
