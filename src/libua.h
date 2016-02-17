// This file is a part of uascript. License is MIT (see LICENSE file)

#ifndef LIBUA_H_
#define LIBUA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "open62541.h"
#include "lua.h"
//#include "compat-5.2.h"

/* Types are a table with a single lightuserdata entry at index 1, that points
   to the UA_TYPES entry */

int ua_type_tostring(lua_State *L);
int ua_type_instantiate(lua_State *L);
void ua_type_push_typetable(lua_State *L, const UA_DataType *type);

int ua_encodebinary(lua_State *L);
int ua_decodebinary(lua_State *L);

/* UA userdata is always of the below type. The original userdata "owns" the
   memory and needs to garbage-collect it. All derived versions (i.e. created
   during a member access) holds the original userdata in their user value
   field. That way, the original data is garbage collected only when all derived
   members are garbage collected before. */
typedef struct {
    const UA_DataType *type;
    void *data;
} ua_data;

int ua_gc(lua_State *L);
int ua_index(lua_State *L);
int ua_newindex(lua_State *L);
int ua_tostring(lua_State *L);
int ua_pairs(lua_State *L);
ua_data * ua_getdata(lua_State *L, int index, const UA_DataType *type);
int ua_get_type(lua_State *L);
void * ua_getmember(void *parent, const UA_DataType *type, const char *membername,
                    const UA_DataType **memberType, size_t **arraylen);

/* Arrays are of a special type and have a different metatable. Arrays cannot be
   created standalone, but are always part of an enclosing type. Indexing arrays
   returns a copy of the element. */
typedef struct {
    const UA_DataType *type;
    size_t *length;
    void **data;
    /* normally, arrays point to the member of ua_data value. but it can also
       carry the data itself */
    size_t local_length;
    void *local_data;
} ua_array;

int ua_array_tostring(lua_State *L);
int ua_array_new(lua_State *L);
int ua_array_index(lua_State *L);
int ua_array_len(lua_State *L);
int ua_array_newindex(lua_State *L);
int ua_array_append(lua_State *L);
int ua_array_pairs(lua_State *L);
ua_array * ua_getarray(lua_State *L, int index);

/* Server */
int ua_server_new(lua_State *L);
int ua_server_gc(lua_State *L);
int ua_server_start(lua_State *L);
int ua_server_iterate(lua_State *L);
int ua_server_stop(lua_State *L);
int ua_server_add_variablenode(lua_State *L);
int ua_server_add_objectnode(lua_State *L);
int ua_server_add_objecttypenode(lua_State *L);
int ua_server_add_referencetypenode(lua_State *L);
int ua_server_add_reference(lua_State *L);
int ua_server_add_methodnode(lua_State *L);
int ua_server_write(lua_State *L);
int ua_server_read(lua_State *L);

/* Client */
int ua_client_new(lua_State *L);
int ua_client_gc(lua_State *L);
int ua_client_connect(lua_State *L);
int ua_client_disconnect(lua_State *L);
int ua_client_service_browse(lua_State *L);
int ua_client_service_browsenext(lua_State *L);
int ua_client_service_read(lua_State *L);
int ua_client_service_write(lua_State *L);
int ua_client_service_call(lua_State *L);
int ua_client_getendpoints(lua_State *L);

/* Function Export */
#ifdef _WIN32
# ifdef UA_DYNAMIC_LINKING
#  ifdef __GNUC__
#   define EXPORT __attribute__ ((dllexport))
#  else
#   define EXPORT __declspec(dllexport)
#  endif
# else
#  ifdef __GNUC__
#   define EXPORT __attribute__ ((dllexport))
#  else
#   define EXPORT __declspec(dllimport)
#  endif
# endif
#else
# if __GNUC__ || __clang__
#  define EXPORT __attribute__ ((visibility ("default")))
# else
#  define EXPORT
# endif
#endif

/* Populate the Module */
int EXPORT luaopen_libua(lua_State *L);

#ifdef __cplusplus
}
#endif

#endif /* LIBUA_H_ */
