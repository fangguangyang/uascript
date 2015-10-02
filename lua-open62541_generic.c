#include "lua-open62541.h"

static int find_typeindex(size_t length, const char *typename) {
    int typeindex;
    for(typeindex = 0; typeindex < UA_TYPES_COUNT; typeindex++) {
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
            return typeindex;
    }
    return -1;
}

static int find_memberindex(const UA_DataType *type, size_t length, const char *membername) {
    int memberindex;
    for(memberindex = 0; memberindex < type->membersSize; memberindex++) {
        UA_Boolean found = UA_TRUE;
        const UA_DataTypeMember *member = &type->members[memberindex];
        if(!member->memberName)
            continue;
        size_t i;
        for(i = 0; i < length; i++) {
            if(!member->memberName[i] ||
               tolower(member->memberName[i]) != membername[i]) {
                found = UA_FALSE;
                break;
            }
        }
        if(i != length)
            continue;
        if(found)
            return memberindex;
    }
    return -1;
}

/* get the pointer to the member and/or the array length */
static void *
memberptr(void *parent, const UA_DataType *type, int memberindex, UA_Int32 **arraylen) {
    void *ptr = parent;
    const UA_DataTypeMember *member;
    for(int i = 0; i < memberindex; i++) {
        member = &type->members[i];
        if(!member->isArray)
            ptr += member->padding + UA_TYPES[member->memberTypeIndex].memSize;
        else
            ptr += (member->padding >> 3) + sizeof(UA_Int32) +
                (member->padding & 0x07) + sizeof(void*);
    }
    member = &type->members[memberindex];
    if(member->isArray) {
        ptr += (member->padding >> 3);
        *arraylen = (UA_Int32*)ptr;
        return ptr + sizeof(UA_Int32) + (member->padding & 0x07);
    }
    *arraylen = NULL;
    return ptr + member->padding;
}

static int ua_new_closure(lua_State *L) {
    int n = lua_gettop(L);
    UA_DataType *type = lua_touserdata(L, lua_upvalueindex(1));
    ua_data *data = lua_newuserdata(L, sizeof(ua_data)); // n+1
    data->type = type;
    data->data = UA_new(type);
    luaL_setmetatable(L, "open62541-data");
    if(type == &UA_TYPES[UA_TYPES_STRING])
        ua_string_new(L, data->data, 1);
    return 1;
}

int ua_new(lua_State *L) {
    if(!lua_isstring(L, 2))
        return luaL_error(L, "Unknown module member");
    size_t length;
    const char *typename = lua_tolstring(L, 2, &length);
    int typeindex = find_typeindex(length, typename);
    if(typeindex < 0)
        return luaL_error(L, "Unknown module member");
    lua_pushlightuserdata(L, (void*)&UA_TYPES[typeindex]);
    lua_pushcclosure(L, &ua_new_closure, 1);
    return 1;
}

int ua_gc(lua_State *L) {
    ua_data *data = luaL_checkudata (L, -1, "open62541-data");
    lua_getuservalue(L, -1);
    /* if a uservalue is attached -> derived data is not deleted */
    if(lua_isnil(L, -1))
        UA_delete(data->data, data->type);
    return 0;
}

int ua_index(lua_State *L) {
    ua_data *data = luaL_checkudata (L, 1, "open62541-data");
    if(!lua_isstring(L, 2))
        return luaL_error(L, "Unknown index");
    size_t length;
    const char *membername = lua_tolstring(L, 2, &length);
    int memberindex = find_memberindex(data->type, length, membername);
    if(memberindex < 0) {
        lua_pushnil(L);
        return 1;
    }

    UA_Int32 *arraylen;
    void *member = memberptr(data->data, data->type, memberindex, &arraylen);
    if(!arraylen) {
        /* scalar type */
        ua_data *memberdata = lua_newuserdata(L, sizeof(ua_data));
        memberdata->type = &UA_TYPES[data->type->members[memberindex].memberTypeIndex];
        memberdata->data = member;
        luaL_setmetatable(L, "open62541-data");
    } else {
        /* array type */
        ua_array *memberdata = lua_newuserdata(L, sizeof(ua_array));
        memberdata->type = &UA_TYPES[data->type->members[memberindex].memberTypeIndex];
        memberdata->data = (void**)member;
        memberdata->length = arraylen;
        luaL_setmetatable(L, "open62541-array");
    }
    
    /* add a user value that prevents gc of the orig userdata */
    lua_newtable(L);
    lua_pushvalue(L, 1); // orig userdata
    lua_rawseti(L, -2, 1);
    lua_setuservalue(L, -2); // set the uservalue of the member data
    return 1;
}

int ua_newindex(lua_State *L) {
    ua_data *parent = luaL_checkudata (L, 1, "open62541-data");
    if(!lua_isstring(L, 2))
        return luaL_error(L, "Index must be a string");
    size_t keylen;
    const char *key = lua_tolstring(L, 2, &keylen);
    ua_data *value = luaL_checkudata (L, 3, "open62541-data");
    int memberindex = find_memberindex(parent->type, keylen, key);
    if(memberindex < 0)
        return luaL_error(L, "Index not found");
    const UA_DataType *membertype = &UA_TYPES[parent->type->members[memberindex].memberTypeIndex];
    if(membertype != value->type)
        return luaL_error(L, "Types don't match %d, %d", membertype->typeIndex, value->type->typeIndex);
    UA_Int32 *arraylen;
    void *member = memberptr(parent->data, parent->type, memberindex, &arraylen);
    if(arraylen)
        return luaL_error(L, "Cannot set arrays yet");
    UA_deleteMembers(member, membertype);
    UA_copy(value->data, member, membertype);
    return 0;
}

static void ua_tostring_recursive(lua_State *L, void* p, const UA_DataType *type, size_t depth);

static void
tostring_array(lua_State *L, void *p, UA_Int32 length, const UA_DataType *type, size_t depth) {
    int n  = lua_gettop(L);
    lua_pushstring(L, "[ ");
    for(UA_Int32 j = 0; j < length; j++) {
        ua_tostring_recursive(L, p + (j * type->memSize), type, depth+1);
        if(j+1 < length)
            lua_pushstring(L, ",\n");
        lua_concat(L, lua_gettop(L)-n);
    }
    lua_pushstring(L, " ]");
    lua_concat(L, lua_gettop(L)-n);
}

static void
ua_tostring_recursive(lua_State *L, void* p, const UA_DataType *type, size_t depth) {
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
        ua_guid_tostring(L, (UA_Guid*)ptr);
        return;
    case UA_TYPES_STRING:
    case UA_TYPES_BYTESTRING:
    case UA_TYPES_XMLELEMENT:
        lua_pushstring(L, "string(\"");
        ua_string_tostring(L, (UA_String*)ptr);
        lua_pushstring(L, "\")");
        lua_concat(L, lua_gettop(L)-n);
        return;
    case UA_TYPES_NODEID:
        ua_nodeid_tostring(L, (UA_NodeId*)ptr);
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

    lua_pushfstring(L, "(%s):\n", type->typeName);
    UA_Byte membersSize = type->membersSize;
    for(size_t i=0;i<membersSize; i++) {
        const UA_DataTypeMember *member = &type->members[i];
        const UA_DataType *memberType = &UA_TYPES[member->memberTypeIndex];
        lua_pushlstring(L, tabs, depth);
        if(member->memberName) {
            lua_pushstring(L, member->memberName);
            lua_pushstring(L, " = ");
        }

        if(member->isArray) {
            ptr += (member->padding >> 3);
            UA_Int32 noElements = *(UA_Int32*)ptr;
            ptr += sizeof(UA_Int32) + (member->padding & 0x07);
            tostring_array(L, *(void**)ptr, noElements, memberType, depth);
            ptr += sizeof(void*);
        } else {
            ptr += member->padding;
            ua_tostring_recursive(L, (void*)ptr, memberType, depth+1);
            ptr += UA_TYPES[member->memberTypeIndex].memSize;
        }
        if(i+1 < membersSize)
            lua_pushstring(L, "\n");
        lua_concat(L, lua_gettop(L)-n);
    }
    lua_concat(L, lua_gettop(L)-n);
}

int ua_tostring(lua_State *L) {
    ua_data *data = luaL_checkudata (L, -1, "open62541-data");
    ua_tostring_recursive(L, data->data, data->type, 0);
    return 1;
}

int ua_array_tostring(lua_State *L) {
    ua_array *array = luaL_checkudata (L, -1, "open62541-array");
    tostring_array(L, *array->data, *array->length, array->type, 0);
    return 1;
}

int ua_array_index(lua_State *L) {
    // lua_pushcfunction(L, ua_array_append);
    // lua_setfield(L, -2, "append");
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(lua_isstring(L, 2)) {
        const char *index = lua_tostring(L, 2);
        if(strcmp(index, "append") == 0) {
            lua_pushcfunction(L, ua_array_append);
            return 1;
        }
    }
    if(!lua_isnumber(L, 2))
        return luaL_error(L, "Unknown index");

    int index = lua_tointeger(L, 2);
    index--; // lua is 1-indexed
    if(index < 0 || index > *array->length)
        return luaL_error(L, "Index out of range");

    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = array->type;
    data->data = UA_new(array->type);
    UA_copy(*array->data + (index * array->type->memSize), data->data, array->type);
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_array_append(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    ua_data *value = luaL_checkudata (L, 2, "open62541-data");
    if(array->type != value->type)
        return luaL_error(L, "Types don't match");
    UA_Int32 newlength = (*array->length) + 1;
    if(newlength <= 0)
        newlength = 1;
    *array->data = realloc(*array->data, newlength * array->type->memSize);
    UA_copy(value->data, *array->data + ((newlength-1) * array->type->memSize), array->type);
    *(array->length) = newlength;
    return 0;
}

int ua_array_len(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(*array->length <= 0)
        lua_pushinteger(L, 0);
    else
        lua_pushinteger(L, *array->length);
    return 1;
}
