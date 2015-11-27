#include "uascript.h"
#include "lualib.h"
#include "lauxlib.h"

/*****************/
/* Type Handling */
/*****************/

/* Guid */
static UA_Guid* parse_guid(lua_State *L, int index) {
    if(!lua_isstring(L, index)) {
        luaL_error(L, "Guid needs to be of the form \"00000000-0000-0000-0000-000000000000\"");
        return NULL;
    }
    size_t arglength;
    const char *arg = lua_tolstring(L, index, &arglength);
    if(arglength != 36) {
        luaL_error(L, "Guid needs to be of the form \"00000000-0000-0000-0000-000000000000\"");
        return NULL;
    }
    UA_Guid *guid = UA_Guid_new();
    guid->data1 = strtoull(arg, NULL, 16);
    guid->data2 = strtoull(&arg[9], NULL, 16);
    guid->data3 = strtoull(&arg[14], NULL, 16);
    UA_Int16 data4_1 = strtoull(&arg[19], NULL, 16);
    guid->data4[0] = data4_1 >> 8;
    guid->data4[1] = data4_1;
    UA_Int64 data4_2 = strtoull(&arg[24], NULL, 16);
    guid->data4[2] = data4_2 >> 40;
    guid->data4[3] = data4_2 >> 32;
    guid->data4[4] = data4_2 >> 24;
    guid->data4[5] = data4_2 >> 16;
    guid->data4[6] = data4_2 >> 8;
    guid->data4[7] = data4_2;
    return guid;
}

/* String */
static UA_String* parse_string(lua_State *L, int index) {
    if(!lua_isstring(L, index)) {
        luaL_error(L, "The argument is no string");
        return NULL;
    }
    UA_String *s = UA_String_new();
    *s = UA_STRING_ALLOC(lua_tostring(L, index));
    return s;
}

/* NodeId */
static UA_NodeId* parse_nodeid(lua_State *L, int index) {
    if(!lua_isnumber(L, index)) {
        luaL_error(L, "The first argument is no integer namespace id");
        return NULL;
    } else if(!lua_isnumber(L, index+1) && !lua_isstring(L, index+1)) {
        luaL_error(L, "Currently, only numeric and string nodeids are implemented");
        return NULL;
    }
    UA_NodeId *id = UA_NodeId_new();
    id->namespaceIndex = lua_tointeger(L, index);
    if(lua_isnumber(L, index+1)) {
        id->identifierType = UA_NODEIDTYPE_NUMERIC;
        id->identifier.numeric = lua_tointeger(L, index+1);
    } else if(lua_isstring(L, index+1)) {
        id->identifierType = UA_NODEIDTYPE_STRING;
        id->identifier.string = UA_String_fromChars(lua_tostring(L, index+1));
    } 
    return id;
}

/* QualifiedName */
static UA_QualifiedName* parse_qualifiedname(lua_State *L, int index) {
    if(!lua_isnumber(L, index)) {
        luaL_error(L, "The first argument is no integer namespace index");
        return NULL;
    } else if(!lua_isstring(L, index+1)) {
        luaL_error(L, "The second argument is no string");
        return NULL;
    }
    UA_QualifiedName *qn = UA_QualifiedName_new();
    qn->namespaceIndex = lua_tointeger(L, index);
    qn->name = UA_STRING_ALLOC(lua_tostring(L, index+1));
    return qn;
}

/* LocalizedText */
static UA_LocalizedText* parse_localizedtext(lua_State *L, int index) {
    if(!lua_isstring(L, index)) {
        luaL_error(L, "The first argument is no string");
        return NULL;
    } else if(!lua_isstring(L, index+1)) {
        luaL_error(L, "The second argument is no string");
        return NULL;
    }
    UA_LocalizedText *lt = UA_LocalizedText_new();
    lt->locale = UA_STRING_ALLOC(lua_tostring(L, index));
    lt->text = UA_STRING_ALLOC(lua_tostring(L, index+1));
    return lt;
}

static void *ua_type_parse(lua_State *L, const UA_DataType *type, int index) {
    if(lua_isnil(L, index))
        return UA_new(type);
    if(!type->builtin) {
        luaL_error(L, "Only builtin types can be instantiated with arguments");
        return NULL;
    }
    switch(type->typeIndex) {
    case UA_TYPES_STRING:
        return parse_string(L, index);
    case UA_TYPES_GUID:
        return parse_guid(L, index);
    case UA_TYPES_NODEID:
        return parse_nodeid(L, index);
    case UA_TYPES_QUALIFIEDNAME:
        return parse_qualifiedname(L, index);
    case UA_TYPES_LOCALIZEDTEXT:
        return parse_localizedtext(L, index);
    }
    luaL_error(L, "Cannot parse the arguments for the data type");
    return NULL;
}

/* General type constructor function. Takes the datatype from its closure. */
int ua_type_instantiate(lua_State *L) {
    lua_pushnil(L); // separator between data from the user and what we add
    lua_rawgeti(L, 1, 1);
    const UA_DataType *type = lua_touserdata(L, -1);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = type;
    data->data = ua_type_parse(L, type, 2);
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_type_tostring(lua_State *L) {
    lua_rawgeti(L, 1, 1);
    const UA_DataType *type = lua_touserdata(L, 2);
    lua_pushstring(L, "ua.types.");
    lua_pushstring(L, type->typeName);
    lua_concat(L, 2);
    return 1;
}

int ua_type_indexerr(lua_State *L) {
    return luaL_error(L, "Type cannot be indexed");
}

void ua_type_push_typetable(lua_State *L, const UA_DataType *type) {
    lua_newtable(L);
    lua_pushlightuserdata(L, (void*)(uintptr_t)type);
    lua_rawseti(L, -2, 1);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    UA_NodeId *typeId = UA_NodeId_new();
    *typeId = type->typeId;
    data->data= typeId;
    data->type = &UA_TYPES[UA_TYPES_NODEID];
    luaL_setmetatable(L, "open62541-data");
    lua_setfield(L, -2, "typeId");
    luaL_setmetatable(L, "open62541-type");
}

int ua_get_type(lua_State *L) {
    const UA_DataType *type = NULL;
    ua_data *data = luaL_checkudata (L, 1, "open62541-data");
    if(data)
        type = data->type;
    else {
        ua_array *array = luaL_checkudata(L, 1, "open62541-array");
        type = array->type;
    }
    if(!type) {
        lua_pushnil(L);
        return 1;
    }
    lua_newtable(L);
    luaL_setmetatable(L, "open62541-type");
    lua_pushlightuserdata(L, (void*)(uintptr_t)type);
    lua_rawseti(L, -2, 1);
    return 1;
}

/**************************/
/* Type Instance Handling */
/**************************/

/* Read the data at the index in the lua stack, replace it with an open62541 data and return a pointer */
ua_data * ua_getdata(lua_State *L, int index) {
    ua_data *data = luaL_testudata(L, index , "open62541-data");
    if(data)
        return data;

    /* try to convert */
    if(lua_isboolean(L, index)) {
        data = lua_newuserdata(L, sizeof(ua_data));
        data->type = &UA_TYPES[UA_TYPES_BOOLEAN];
        data->data = UA_Boolean_new();
        *(UA_Boolean*)data->data = lua_toboolean(L, index);
        luaL_setmetatable(L, "open62541-data");
        lua_replace(L, index);
    } else if(lua_isnumber(L, index)) {
        data = lua_newuserdata(L, sizeof(ua_data));
        data->type = &UA_TYPES[UA_TYPES_INT32];
        data->data = UA_Int32_new();
        *(UA_Int32*)data->data = lua_tonumber(L, index);
        luaL_setmetatable(L, "open62541-data");
        lua_replace(L, index);
    } else if(lua_isstring(L, index)) {
        data = lua_newuserdata(L, sizeof(ua_data));
        data->type = &UA_TYPES[UA_TYPES_STRING];
        data->data = parse_string(L, index);
        luaL_setmetatable(L, "open62541-builtin");
        lua_replace(L, index);
    }
    return data;
}

static void
push_lowercase(lua_State *L, const char *str) {
    char out[512];
    size_t i;
    for(i = 0; i<512; i++) {
        out[i] = tolower(str[i]);
        if(!str[i])
            break;
    }
    lua_pushlstring(L, out, i);
}

/* Get the index of the member with the given name */
static int
find_memberindex(const UA_DataType *type, const char *membername) {
    int memberindex;
    int length = strlen(membername);
    for(memberindex = 0; memberindex < type->membersSize; memberindex++) {
        UA_Boolean found = UA_TRUE;
        const UA_DataTypeMember *member = &type->members[memberindex];
        if(!member->memberName)
            continue;
        if(strlen(member->memberName) != length)
            continue;
        for(size_t i = 0; i < length; i++) {
            if(tolower(member->memberName[i]) != membername[i]) {
                found = UA_FALSE;
                break;
            }
        }
        if(found)
            return memberindex;
    }
    return -1;
}

/* get the pointer to the member and the array length if applicable */
static void *
memberptr(void *parent, const UA_DataType *type, int memberindex, size_t **arraylen) {
    uintptr_t ptr = (uintptr_t)parent;
    const UA_DataTypeMember *member;
    for(int i = 0; i < memberindex; i++) {
        member = &type->members[i];
        if(!member->isArray)
            ptr += member->padding + UA_TYPES[member->memberTypeIndex].memSize;
        else
            ptr += member->padding + sizeof(size_t) + sizeof(void*);
    }
    member = &type->members[memberindex];
    if(member->isArray) {
        ptr += member->padding;
        *arraylen = (size_t*)ptr;
        ptr += sizeof(size_t);
    } else {
        *arraylen = NULL;
        ptr += member->padding;
    }
    return (void*)ptr;
}

int ua_gc(lua_State *L) {
    ua_data *data = ua_getdata(L, -1);
    if(!data)
        return 0;
    lua_getuservalue(L, -1);
    /* derived data is not deleted, tagged with a uservalue */
    if(lua_isnil(L, -1))
        UA_delete(data->data, data->type);
    return 0;
}

static int
ua_getmemberindex(lua_State *L, ua_data *data, int memberindex) {
    size_t *arraylen;
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

static int
ua_variant_index(lua_State *L, UA_Variant *v, const char *key) {
    if(strcmp(key, "value") == 0) {
        if(UA_Variant_isScalar(v)) {
            ua_data *data = lua_newuserdata(L, sizeof(ua_data));
            data->type = v->type;
            data->data = UA_new(v->type);
            UA_copy(v->data, data->data, v->type);
            luaL_setmetatable(L, "open62541-data");
            return 1;
        } else {
            ua_array *array = lua_newuserdata(L, sizeof(ua_array));
            array->type = v->type;
            array->data = &array->local_data;
            array->length = &array->local_length;
            UA_Array_copy(v->data, v->arrayLength, &array->local_data, v->type);
            array->local_length = v->arrayLength;
            luaL_setmetatable(L, "open62541-array");
            return 1;
        }
    }
    return luaL_error(L, "Cannot get this Variant content");
}

static int
ua_datavalue_index(lua_State *L, UA_DataValue *v, const char *key) {
    if(strcmp(key, "value") == 0) {
        if(!v->hasValue) {
            lua_pushnil(L);
            return 1;
        }
        UA_Variant *copy = UA_Variant_new();
        UA_Variant_copy(&v->value, copy);
        ua_data *data = lua_newuserdata(L, sizeof(ua_data));
        data->type = &UA_TYPES[UA_TYPES_VARIANT];
        data->data = copy;
        luaL_setmetatable(L, "open62541-data");
        return 1;
    }
    lua_pushnil(L);
    return 1;
}

int ua_index(lua_State *L) {
    ua_data *data = luaL_checkudata (L, -2, "open62541-data");
    if(!lua_isstring(L, -1))
        return luaL_error(L, "Unknown index");
    const char *key = lua_tostring(L, -1);
    if(data->type == &UA_TYPES[UA_TYPES_VARIANT])
        return ua_variant_index(L, data->data, key);
    else if(data->type == &UA_TYPES[UA_TYPES_DATAVALUE])
        return ua_datavalue_index(L, data->data, key);
    int memberindex = find_memberindex(data->type, key);
    if(memberindex < 0) {
        lua_pushnil(L);
        return 1;
    }
    return ua_getmemberindex(L, data, memberindex);
}

static int
ua_variant_newindex(lua_State *L, UA_Variant *v, const char *key, size_t index) {
    if(strcmp(key, "value") != 0)
       return luaL_error(L, "Can only set the value for now");
    ua_data *data = ua_getdata(L, index);
    if(data) {
        UA_Variant_deleteMembers(v);
        UA_Variant_setScalarCopy(v, data->data, data->type);
        return 0;
    }
    ua_array *array = luaL_checkudata (L, index, "open62541-array");
    if(array) {
        UA_Variant_deleteMembers(v);
        UA_Variant_setArrayCopy(v, *array->data, *array->length, array->type);
        return 0;
    }
    return luaL_error(L, "Cannot set this data type to the variant");
}

int ua_newindex(lua_State *L) {
    ua_data *parent = luaL_checkudata (L, 1, "open62541-data");
    if(!lua_isstring(L, 2))
        return luaL_error(L, "Index must be a string");
    /* variants have a special newindex */
    const char *key = lua_tostring(L, 2);
    if(parent->type == &UA_TYPES[UA_TYPES_VARIANT])
        return ua_variant_newindex(L, parent->data, key, 3);
    int memberindex = find_memberindex(parent->type, key);
    if(memberindex < 0)
        return luaL_error(L, "Index not found");

    const UA_DataType *membertype = &UA_TYPES[parent->type->members[memberindex].memberTypeIndex];
    size_t *arraylen;
    void *member = memberptr(parent->data, parent->type, memberindex, &arraylen);

    if(arraylen) {
        ua_array *array = luaL_checkudata(L, 3, "open62541-array");
        if(!array)
            return luaL_error(L, "The value is not an array");
        if(membertype != array->type)
            return luaL_error(L, "Types don't match");
        UA_Array_delete(*(void**)member, *arraylen, membertype);
        UA_Array_copy(*array->data, *array->length, member, membertype);
        *arraylen = *array->length;
        return 0;
    }
    
    ua_data *data = ua_getdata(L, 3);
    if(!data)
        return luaL_error(L, "Types don't match");
    if(data->type == &UA_TYPES[UA_TYPES_INT32]) {
        UA_Int32 n = *(UA_Int32*)data->data;
        if(membertype->builtin) {
            switch(membertype->typeIndex) {
            case UA_TYPES_BOOLEAN:
                *(UA_Boolean*)member = n;
                return 0;
            case UA_TYPES_SBYTE:
            case UA_TYPES_BYTE:
                *(UA_Byte*)member = n;
                return 0;
            case UA_TYPES_INT16:
            case UA_TYPES_UINT16:
                *(UA_Int16*)member = n;
                return 0;
            case UA_TYPES_INT32:
            case UA_TYPES_UINT32:
            case UA_TYPES_STATUSCODE:
                *(UA_Int32*)member = n;
                return 0;
            case UA_TYPES_INT64:
            case UA_TYPES_UINT64:
            case UA_TYPES_DATETIME:
                *(UA_Int64*)member = n;
                return 0;
            }
        }
    }
    if(membertype != data->type)
        return luaL_error(L, "Types don't match");
    UA_deleteMembers(member, membertype);
    UA_copy(data->data, member, membertype);
    return 0;
}

int ua_len(lua_State *L) {
    ua_data *data = luaL_checkudata (L, 1, "open62541-data");
    if(!data)
        return luaL_error(L, "Cannot get the data");
    lua_pushinteger(L, data->type->membersSize);
    return 1;
}

static int
ua_iterate(lua_State *L) {
    ua_data *data = luaL_checkudata (L, 1, "open62541-data");
    if(!data->type->builtin)
        return 0;
    int memberindex;
    if(lua_isnil(L, 2))
        memberindex = 0;
    else if(lua_isstring(L, 2)) {
        memberindex = find_memberindex(data->type, lua_tostring(L, 2));
        if(memberindex < 0)
            return luaL_error(L, "No valid index");
        memberindex++;
    } else
       return luaL_error(L, "Can only iterate over string indices");
    if(memberindex >= data->type->membersSize)
        return 0;
    push_lowercase(L, data->type->members[memberindex].memberName);
    ua_getmemberindex(L, data, memberindex);
    return 2;
}
        
int ua_pairs(lua_State *L) {
    ua_data *data = luaL_checkudata (L, 1, "open62541-data");
    lua_pushcfunction(L, ua_iterate);
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    return 3;
}


static int ua_tostring_builtin(lua_State *L, const UA_DataType *type, void *data, size_t level);
static int ua_tostring_structured(lua_State *L, const UA_DataType *type, void *data, size_t level);
static int ua_tostring_array(lua_State *L, const UA_DataType *type, void *data, size_t len, size_t level);

static int
ua_tostring_variant(lua_State *L, const UA_Variant *v, size_t level) {
    int pushed = 1; 
    lua_pushstring(L, "ua.types.Variant:\n");

    for(size_t j = 0; j < level; j++)
        lua_pushstring(L, "  ");
    pushed += level;

    lua_pushstring(L, "value = ");
    pushed++;
    if(!v->type) {
        lua_pushstring(L, "nil");
        pushed++;
    } else if(UA_Variant_isScalar(v)) {
        if(v->type->builtin)
            pushed += ua_tostring_builtin(L, v->type, v->data, level+1);
        else
            pushed += ua_tostring_structured(L, v->type, v->data, level+1);
    } else
        pushed += ua_tostring_array(L, v->type, v->data, v->arrayLength, level+1);

    lua_pushstring(L, "\n");
    for(size_t j = 0; j < level; j++)
        lua_pushstring(L, "  ");
    lua_pushstring(L, "arrayDimensions = ");
    pushed += 2 + level + ua_tostring_array(L, &UA_TYPES[UA_TYPES_UINT32], v->arrayDimensions, v->arrayDimensionsSize, level+1);
    lua_concat(L, pushed);
    return 1;
}

static int
ua_tostring_datavalue(lua_State *L, const UA_DataValue *v, size_t level) {
    int pushed = 0; 
    if(v->hasValue) {
        for(size_t j = 0; j < level; j++)
            lua_pushstring(L, "  ");
        lua_pushstring(L, "value = ");
        pushed += level + 1 + ua_tostring_variant(L, &v->value, level+1);
    } else {
        for(size_t j = 0; j < level; j++)
            lua_pushstring(L, "  ");
        lua_pushstring(L, "value = nil");
        pushed += level + 1;
    }
    lua_concat(L, pushed);
    pushed = 1;

    /* if(v->hasStatus) { */

    /* } */

    return pushed;
}

static int
ua_tostring_array(lua_State *L, const UA_DataType *type, void *data, size_t len, size_t level) {
    int pushed = 1;
    if(len > 0)
        lua_pushfstring(L, "ua.Array(ua.types.%s,%d):\n", type->typeName, (int)len);
    else {
        if(data > 0)
            lua_pushfstring(L, "ua.Array(ua.types.%s,0):", type->typeName);
        else
            lua_pushfstring(L, "ua.Array(ua.types.%s,-1):", type->typeName);
    }
    uintptr_t p = (uintptr_t)data;
    for(int i = 0; i < len; i++) {
        for(size_t j = 0; j < level; j++)
            lua_pushstring(L, "  ");
        pushed += level;
        lua_pushfstring(L, "%d = ", i+1);
        pushed += 1;
        if(type->builtin)
            pushed += ua_tostring_builtin(L, type, (void*)p, level+1);
        else
            pushed += ua_tostring_structured(L, type, (void*)p, level+1);
        p += type->memSize;
        if(i + 1 < len) {
            lua_pushstring(L, "\n");
            pushed++;
        }
        lua_concat(L, pushed);
        pushed = 1;
    }
    return 1;
}

static int
ua_tostring_structured(lua_State *L, const UA_DataType *type, void *data, size_t level) {
    int pushed = 1;
    lua_pushfstring(L, "ua.types.%s:\n", type->typeName);
    for(size_t i = 0; i < type->membersSize; i++) {
        for(size_t j = 0; j < level; j++)
            lua_pushstring(L, "  ");
        pushed += level;
        const UA_DataTypeMember *member = &type->members[i];
        const UA_DataType *typelists[2] = { UA_TYPES, &type[-type->typeIndex] };
        const UA_DataType *memberType = &typelists[!member->namespaceZero][member->memberTypeIndex];
        size_t *arraylen;
        void *memberp = memberptr(data, type, i, &arraylen);
        push_lowercase(L, member->memberName);
        lua_pushstring(L, " = ");
        pushed += 2;
        if(arraylen)
            pushed += ua_tostring_array(L, memberType, *(void**)memberp, *arraylen, level+1);
        else if(memberType->builtin)
            pushed += ua_tostring_builtin(L, memberType, memberp, level+1);
        else
            pushed += ua_tostring_structured(L, memberType, memberp, level+1);
        if(i + 1 < type->membersSize) {
            lua_pushstring(L, "\n");
            pushed++;
        }
        lua_concat(L, pushed);
        pushed = 1;
    }
    return 1;
}

static int
ua_tostring_builtin(lua_State *L, const UA_DataType *type, void *data, size_t level) {
    char out[37];
    switch(type->typeIndex) {
    case UA_TYPES_BOOLEAN:
        if(*(UA_Boolean*)data)
            lua_pushstring(L, "true");
        else
            lua_pushstring(L, "false");
        return 1;
    case UA_TYPES_SBYTE:
        snprintf(out, 40, "%hhi", *(UA_SByte*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_BYTE:
        snprintf(out, 40, "%hhu", *(UA_Byte*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_INT16:
        snprintf(out, 40, "%hi", *(UA_Int16*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_UINT16:
        snprintf(out, 40, "%hu", *(UA_UInt16*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_INT32:
        snprintf(out, 40, "%i", *(UA_Int32*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_UINT32:
        snprintf(out, 40, "%u", *(UA_UInt32*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_INT64:
        snprintf(out, 40, "%li", *(UA_Int64*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_UINT64:
        snprintf(out, 40, "%lu", *(UA_UInt64*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_STATUSCODE:
        snprintf(out, 40, "0x%08x", *(UA_StatusCode*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_FLOAT:
        snprintf(out, 40, "%f", *(UA_Float*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_DOUBLE:
        snprintf(out, 40, "%lf", *(UA_Double*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_GUID: {
        UA_Guid *id = (UA_Guid*)data;
        snprintf(out, 37, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                 id->data1, id->data2, id->data3, id->data4[0], id->data4[1],
                 id->data4[2], id->data4[3], id->data4[4], id->data4[5], id->data4[6], id->data4[7]);
        lua_pushstring(L, out);
        return 1;
    }
    case UA_TYPES_STRING:
    case UA_TYPES_BYTESTRING:
    case UA_TYPES_XMLELEMENT: {
        UA_String *s = (UA_String*)data;
        if(s->data == NULL) {
            if(level == 0)
                lua_pushfstring(L, "");
            else
                lua_pushfstring(L, "ua.types.%s()", type->typeName);
            return 1;
        } else {
            if(level == 0) {
                lua_pushlstring(L, s->data, s->length);
                return 1;
            } else {
                lua_pushstring(L, "\"");
                lua_pushlstring(L, s->data, s->length);
                lua_pushstring(L, "\"");
                return 3;
            }
        }
    }
    case UA_TYPES_NODEID: {
        const UA_NodeId *id = (const UA_NodeId*)data;
        if(id->identifierType == UA_NODEIDTYPE_NUMERIC)
            lua_pushfstring(L, "NodeId(ns=%d,i=%d)", id->namespaceIndex, id->identifier.numeric);
        else if(id->identifierType == UA_NODEIDTYPE_STRING) {
            char out[512];
            snprintf(out, 512, "NodeId(ns=%i,s=%.*s)", id->namespaceIndex,
                     id->identifier.string.length, id->identifier.string.data);
            lua_pushstring(L, out);
        } else
            lua_pushstring(L, "NodeId(unknown)");
        return 1;
    }
    case UA_TYPES_QUALIFIEDNAME:
    case UA_TYPES_LOCALIZEDTEXT:
    case UA_TYPES_EXPANDEDNODEID:
        return ua_tostring_structured(L, type, data, level);
    case UA_TYPES_VARIANT:
        return ua_tostring_variant(L, data, level);
    case UA_TYPES_DATAVALUE:
        return ua_tostring_datavalue(L, data, level);
    default:
        lua_pushfstring(L, "ua.types.%s", type->typeName);
        return 1;
    }
    return 1;
}

int ua_tostring(lua_State *L) {
    ua_data *data = luaL_checkudata (L, -1, "open62541-data");
    if(!data)
        return luaL_error(L, "Cannot print this value");
    if(data->type->builtin)
        return ua_tostring_builtin(L, data->type, data->data, 0);
    int pushed = ua_tostring_structured(L, data->type, data->data, 0);
    lua_concat(L, pushed);
    return 1;
}

/******************/
/* Array Handling */
/******************/

int ua_array_tostring(lua_State *L) {
    ua_array *array = luaL_checkudata (L, -1, "open62541-array");
    if(!array)
        return luaL_error(L, "Cannot print this value");
    return ua_tostring_array(L, array->type, *array->data, *array->length, 0);
}

int ua_array_new(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "open62541-type");
    if(!lua_getmetatable(L, 1) || !lua_rawequal(L, -1, -2))
        return luaL_error(L, "The first argument is not a type");
    lua_rawgeti(L, 1, 1);
    const UA_DataType *type = lua_touserdata(L, -1);

    if(!lua_isnumber(L, 2))
        return luaL_error(L, "The second argument is not a valid array size");

    int size = lua_tointeger(L, 2);
    ua_array *array = lua_newuserdata(L, sizeof(ua_array));
    array->type = type;
    array->data = &array->local_data;
    array->length = &array->local_length;
    if(size < 0) {
        array->local_length = 0;
        array->local_data = NULL;
    } else {
        array->local_length = size;
        array->local_data = UA_Array_new(size, type);
    }
    luaL_setmetatable(L, "open62541-array");
    return 1;
}

int ua_array_index(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    int index = lua_tointeger(L, 2);
    index--; // lua is 1-indexed
    if(index < 0 || index >= *array->length)
        return luaL_error(L, "Index out of range");
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = array->type;
    data->data = UA_new(array->type);
	uintptr_t ptr = *array->data;
    UA_copy(ptr + (index * array->type->memSize), data->data, array->type);
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_array_newindex(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(!array)
        return luaL_error(L, "Not an array");
    if(!lua_isnumber(L, 2))
        return luaL_error(L, "Not an array index");
    int index = lua_tointeger(L, 2) - 1;

    /* removing an entry == set to nil */
    if(lua_isnil(L, 3)) {
        if(index < 0 || index >= *array->length)
            return luaL_error(L, "Cannot remove an entry out of the range");
        if(*array->length == 1) {
            UA_Array_delete(*array->data, 1, array->type);
            *array->data = UA_EMPTY_ARRAY_SENTINEL;
            *array->length = 0;
            return 0;
        }
        uintptr_t arr = (uintptr_t)*array->data;
        UA_deleteMembers((void*)(arr + (array->type->memSize*index)), array->type);
        for(size_t i = index+1; i < *array->length; i++)
            memcpy((void*)(arr + (array->type->memSize*(i-1))), (void*)(arr + (array->type->memSize*i)), array->type->memSize);
        *array->length -= 1;
        return 0;
    }

    ua_data *data = luaL_checkudata(L, 3, "open62541-data");
    if(!data || data->type != array->type)
        return luaL_error(L, "The value is of the wrong type");

    /* insert an entry */
    if(index == *array->length) {
        size_t newsize = *array->length + 1;
        if(*array->data == UA_EMPTY_ARRAY_SENTINEL) 
            *array->data = malloc(array->type->memSize);
        else
            *array->data = realloc(*array->data, newsize * array->type->memSize);
        *array->length = newsize;
		uintptr_t ptr = *array->data;
        UA_copy(data->data, ptr + (((newsize-1) * array->type->memSize)), array->type);
        return 0;
    }

    /* overwrite an entry */
    if(index < 0 || index >= *array->length)
        return luaL_error(L, "The index is out of range");
	uintptr_t ptr = *array->data;
    UA_deleteMembers(ptr + (index * array->type->memSize), array->type);
    UA_copy(data->data, ptr + (index * array->type->memSize), array->type);
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

static int
ua_array_iterate(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(!array)
        return luaL_error(L, "Not a open62541 datatype");
    int index;
    if(lua_isnil(L, 2))
        index = 0;
    else if(lua_isnumber(L, 2))
        index = lua_tonumber(L, 2);
    else
        return luaL_error(L, "Can only iterate over numeric indices");
    if(index < 0 || index >= *array->length)
        return 0;
    lua_pushnumber(L, index+1);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = array->type;
    data->data = UA_new(array->type);
	uintptr_t ptr = *array->data;
    UA_copy(ptr + (index * array->type->memSize), data->data, array->type);
    luaL_setmetatable(L, "open62541-data");
    return 2;
}
        
int ua_array_pairs(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(!array)
        return luaL_error(L, "Not a open62541 datatype");
    lua_pushcfunction(L, ua_array_iterate);
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    return 3;
}
