#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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
    return 1;
}

static int string_tostring(lua_State *L, const UA_String *s) {
    if(s->length <= 0)
        lua_pushlstring(L, NULL, 0);
    else
        lua_pushlstring(L, s->data, s->length);
    return 1;
}

/* Guid */
static int guid_new(lua_State *L) {
    const char *arg;
    if(lua_gettop(L) > 0) {
        if(!lua_isstring(L, 1))
            return luaL_error(L, "The argument needs to be a string of the form \"00000000-0000-0000-0000-000000000000\"");
        size_t arglength;
        arg = lua_tolstring(L, 1, &arglength);
        if(arglength != 36)
            return luaL_error(L, "The guid string has not the right format: \"00000000-0000-0000-0000-000000000000\"");
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

static int guid_tostring(lua_State *L, const UA_Guid *id) {
    char out[37];
    int size = snprintf(out, 37, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                        id->data1, id->data2, id->data3, id->data4[0], id->data4[1],
                        id->data4[2], id->data4[3], id->data4[4], id->data4[5], id->data4[6], id->data4[7]);
    lua_pushlstring(L, out, size);
    return 1;
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

static void nodeid_tostring(lua_State *L, const UA_NodeId *id) {
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

/*****************/
/* QualifiedName */
/*****************/

static int qualifiedname_new(lua_State *L) {
    if(lua_gettop(L) == 2) {
        if(!lua_isnumber(L, 1))
            return luaL_error(L, "The first argument is no integer namespace index");
        if(!lua_isstring(L, 2))
            return luaL_error(L, "The second argument is no string");
    } else if(lua_gettop(L) != 0)
        return luaL_error(L, "Qualifiedname takes none or two arguments. An integer namespace index and a string.");

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

/**************************/
/* Generic Type Functions */
/**************************/

/* open62541 datatypes are represented as userdata. They contain a user value
   (table) with a lightuserdata entry at index 1. This points to the
   UA_Datatype. */

static int generic_new(lua_State *L) {
    int n = lua_gettop(L);
    UA_DataType *type = lua_touserdata(L, lua_upvalueindex(1));
    void *data = lua_newuserdata(L, type->memSize); // n+1
    UA_init(data, type);
    luaL_setmetatable(L, "open62541-type");
    lua_newtable(L); // n+2
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_rawseti(L, n+2, 1);
    lua_setuservalue(L, n+1);
    return 1;
}

/* __index metamethod for the 'c' table (stack: 1 = table 'c', 2 = desired index) */
static int generic_new_closure(lua_State *L) {
    if(!lua_isstring(L, 2))
        return luaL_error(L, "Unknown module member");
    size_t length;
    const char *typename = lua_tolstring(L, 2, &length);
    size_t typeindex;
    for(typeindex = 0;typeindex < UA_TYPES_COUNT; typeindex++) {
        const UA_DataType *type = &UA_TYPES[typeindex];
        UA_Boolean found = UA_TRUE;
        size_t i;
        for(i = 0; i < length; i++) {
            if(!type->typeName[i] || tolower(type->typeName[i]) != typename[i]) {
                found = UA_FALSE;
                break;
            }
        }
        if(i != length)
            continue;
        if(found)
            break;
    }
    if(typeindex == UA_TYPES_COUNT)
        return luaL_error(L, "Unknown module member");
    lua_pushlightuserdata(L, (void*)&UA_TYPES[typeindex]);
    lua_pushcclosure(L, &generic_new, 1);
    return 1;
}

/* pushes new strings always to the top */
void generic_tostring_recursive(lua_State *L, void* p, const UA_DataType *type, size_t depth) {
    int n = lua_gettop(L);
    char tabs[20];
    for(size_t i = 0; i < depth; i++) {
        tabs[i] = '\t';
    }

    uintptr_t ptr = (uintptr_t)p;
    char out[20];
    size_t outlength;

    switch(type->typeIndex) {
    case UA_TYPES_BOOLEAN:
        if(*(UA_Boolean*)ptr)
            lua_pushstring(L, "true");
        else
            lua_pushstring(L, "false");
        return;
    case UA_TYPES_SBYTE:
        outlength = snprintf(out, 19, "%hhi", *(UA_SByte*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_BYTE:
        outlength = snprintf(out, 19, "%hhu", *(UA_Byte*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_INT16:
        outlength = snprintf(out, 19, "%hi", *(UA_Int16*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_UINT16:
        outlength = snprintf(out, 19, "%hu", *(UA_UInt16*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_INT32:
        outlength = snprintf(out, 19, "%i", *(UA_Int32*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_UINT32:
        outlength = snprintf(out, 19, "%u", *(UA_UInt32*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_INT64:
        outlength = snprintf(out, 19, "%li", *(UA_Int64*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_UINT64:
        outlength = snprintf(out, 19, "%lu", *(UA_UInt64*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_STATUSCODE:
        outlength = snprintf(out, 19, "0x%08x", *(UA_StatusCode*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_FLOAT:
        outlength = snprintf(out, 19, "%f", *(UA_Float*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_DOUBLE:
        outlength = snprintf(out, 19, "%lf", *(UA_Double*)ptr);
        lua_pushlstring(L, out, outlength);
        return;
    case UA_TYPES_DATETIME:
        lua_pushstring(L, type->typeName);
        return;
    case UA_TYPES_GUID:
        guid_tostring(L, (UA_Guid*)ptr);
        return;
    case UA_TYPES_STRING:
    case UA_TYPES_BYTESTRING:
    case UA_TYPES_XMLELEMENT:
        lua_pushstring(L, "string(\"");
        string_tostring(L, (UA_String*)ptr);
        lua_pushstring(L, "\")");
        return;
    case UA_TYPES_NODEID:
        nodeid_tostring(L, (UA_NodeId*)ptr);
        return;
    case UA_TYPES_EXPANDEDNODEID:
        lua_pushstring(L, type->typeName);
        return;
    case UA_TYPES_LOCALIZEDTEXT:
        lua_pushstring(L, type->typeName);
        return;
    case UA_TYPES_EXTENSIONOBJECT:
        lua_pushstring(L, type->typeName);
        return;
    case UA_TYPES_DATAVALUE:
        lua_pushstring(L, type->typeName);
        return;
    case UA_TYPES_VARIANT:
        lua_pushstring(L, type->typeName);
        return;
    case UA_TYPES_DIAGNOSTICINFO:
        lua_pushstring(L, type->typeName);
        return;
    }

    lua_pushstring(L, "{");
    if(depth > 0)
        lua_pushstring(L, "\n");

    UA_Byte membersSize = type->membersSize;
    for(size_t i=0;i<membersSize; i++) {
        const UA_DataTypeMember *member = &type->members[i];
        const UA_DataType *memberType = &UA_TYPES[member->memberTypeIndex];
        lua_pushlstring(L, tabs, depth);
        lua_pushstring(L, ".");
        lua_pushstring(L, memberType->typeName);
        lua_pushstring(L, " = ");

        if(member->isArray) {
            ptr += (member->padding >> 3);
            UA_Int32 noElements = *(UA_Int32*)ptr;
            ptr += sizeof(UA_Int32) + (member->padding & 0x07);
            void *elem = (void*)ptr;
            lua_pushstring(L, "[");
            for(int j = 0; j < noElements; j++) {
                generic_tostring_recursive(L, elem + (j * memberType->memSize), memberType, depth+1);
                lua_pushstring(L, ",\n");
                lua_concat(L, lua_gettop(L)-n);
            }
            lua_pushstring(L, "]");
            ptr += sizeof(void*);
        } else {
            ptr += member->padding;
            generic_tostring_recursive(L, (void*)ptr, memberType, depth+1);
            ptr += UA_TYPES[member->memberTypeIndex].memSize;
        }
        if(i+1 < membersSize)
            lua_pushstring(L, "\n");
        lua_concat(L, lua_gettop(L)-n);
    }

    lua_pushstring(L, "}");
    lua_concat(L, lua_gettop(L)-n);
}

static int generic_tostring(lua_State *L) {
    void *data = luaL_checkudata (L, -1, "open62541-type");
    lua_getuservalue(L, -1);
    lua_rawgeti(L, -1, 1);
    UA_DataType *type = lua_touserdata(L, -1);
    generic_tostring_recursive(L, data, type, 0);
    return 1;
}

static int generic_gc(lua_State *L) {
    void *data = luaL_checkudata (L, -1, "open62541-type");
    lua_getuservalue(L, -1);
    lua_rawgeti(L, -1, 1);
    UA_DataType *type = lua_touserdata(L, -1);
    UA_deleteMembers(data, type);
    return 0;
}

/***********************/
/* Populate the Module */
/***********************/

static const struct luaL_Reg open62541 [] = {
    {NULL, NULL} /* sentinel */
};

int luaopen_open62541(lua_State *L) {
    /* metatable for types */
    luaL_newmetatable(L, "open62541-type");
    lua_pushcfunction(L, generic_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, generic_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    /* create the module */
    luaL_newlib(L, open62541);

    /* if no member in the module is found, try to create a type-new function */
    lua_newtable(L);
    lua_pushcfunction(L, generic_new_closure);
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
    return 1;
}
