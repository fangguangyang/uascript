#include "lua-open62541.h"

/* String */
void ua_string_new(lua_State *L, UA_String *s, int index) {
    if(!lua_isstring(L, index))
        luaL_error(L, "Argument is not a string");

    size_t length;
    const char *array = lua_tolstring(L, index, &length);
    *s = UA_String_fromChars(array);
}

void ua_string_tostring(lua_State *L, const UA_String *s) {
    if(s->length <= 0)
        lua_pushlstring(L, NULL, 0);
    else
        lua_pushlstring(L, s->data, s->length);
}

/* Guid */
int ua_guid_new(lua_State *L) {
    const char *arg;
    if(lua_gettop(L) > 0) {
        if(!lua_isstring(L, 1))
            return luaL_error(L, "Guid needs to be of the form \"00000000-0000-0000-0000-000000000000\"");
        size_t arglength;
        arg = lua_tolstring(L, 1, &arglength);
        if(arglength != 36)
            return luaL_error(L, "Guid needs to be of the form \"00000000-0000-0000-0000-000000000000\"");
    }
    UA_Guid *id = lua_newuserdata(L, sizeof(UA_Guid));
    UA_Guid_init(id);
    if(lua_gettop(L) > 1) {
        id->data1 = strtoull(arg, NULL, 16);
        id->data2 = strtoull(&arg[9], NULL, 16);
        id->data3 = strtoull(&arg[14], NULL, 16);
        UA_Int16 data4_1 = strtoull(&arg[19], NULL, 16);
        id->data4[0] = data4_1 >> 8;
        id->data4[1] = data4_1;
        UA_Int64 data4_2 = strtoull(&arg[24], NULL, 16);
        id->data4[2] = data4_2 >> 40;
        id->data4[3] = data4_2 >> 32;
        id->data4[4] = data4_2 >> 24;
        id->data4[5] = data4_2 >> 16;
        id->data4[6] = data4_2 >> 8;
        id->data4[7] = data4_2;
    }
    return 1;
}

void ua_guid_tostring(lua_State *L, const UA_Guid *id) {
    char out[37];
    int size = snprintf(out, 37, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        id->data1, id->data2, id->data3, id->data4[0], id->data4[1],
                        id->data4[2], id->data4[3], id->data4[4], id->data4[5], id->data4[6], id->data4[7]);
    lua_pushlstring(L, out, size);
}

/* NodeId */
int ua_nodeid_new(lua_State *L) {
    if(lua_gettop(L) != 2)
        return luaL_error(L, "nodeid takes two arguments");
    if(!lua_isnumber(L, 1))
        return luaL_error(L, "The first argument is no integer namespace id");
    if(!lua_isnumber(L, 2) && !lua_isstring(L, 2))
        return luaL_error(L, "Currently, only numeric and string nodeids are implemented");

    UA_NodeId *id = lua_newuserdata(L, sizeof(UA_NodeId));
    UA_NodeId_init(id);

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

void ua_nodeid_tostring(lua_State *L, const UA_NodeId *id) {
    char out[512];
    int size;
    if(id->identifierType == UA_NODEIDTYPE_NUMERIC)
        size = snprintf(out, 512, "nodeid(ns=%i,i=%i)", id->namespaceIndex, id->identifier.numeric);
    else if(id->identifierType == UA_NODEIDTYPE_STRING)
        size = snprintf(out, 512, "nodeid(ns=%i,s=%.*s)", id->namespaceIndex,
                        id->identifier.string.length, id->identifier.string.data);
    else
        lua_pushstring(L, "Unknown NodeId type");
    lua_pushlstring(L, out, size);
}

/* QualifiedName */
int ua_qualifiedname_new(lua_State *L) {
    if(lua_gettop(L) == 2) {
        if(!lua_isnumber(L, 1))
            return luaL_error(L, "The first argument is no integer namespace index");
        if(!lua_isstring(L, 2))
            return luaL_error(L, "The second argument is no string");
    } else if(lua_gettop(L) != 0)
        return luaL_error(L, "Qualifiedname takes none or two arguments");

    UA_QualifiedName *qn = lua_newuserdata(L, sizeof(UA_QualifiedName));
    UA_QualifiedName_init(qn);
    if(lua_gettop(L) == 1)
        return 1;

    qn->namespaceIndex = lua_tointeger(L, 1);
    size_t length;
    const char *array = lua_tolstring(L, 2, &length);
    qn->name = UA_String_fromChars(array);
    return 1;
}
