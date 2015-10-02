#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "lua5.2/lua.h"
#include "lua5.2/lauxlib.h"
#include "lua5.2/lualib.h"

#include "open62541.h"

/* Type Handling */
/* UA userdata is always of the below type. The original userdata "owns" the
   memory and needs to garbage-collect it. All derived versions (i.e. created
   during a member access) holds the original userdata in their user value
   field. That way, the original data is garbage collected only when all derived
   members are garbage collected before.
 */
typedef struct {
    const UA_DataType *type;
    void *data;
} ua_data;

/* Arrays are of a special type and have a different metatable. Arrays cannot be
   created standalone, but are always part of an enclosing type. Indexing arrays
   returns a copy of the element. */
typedef struct {
    const UA_DataType *type;
    UA_Int32 *length;
    void **data;
} ua_array;

/* String */
void ua_string_new(lua_State *L, UA_String *s, int index);
void ua_string_tostring(lua_State *L, const UA_String *s);

/* Guid */
int ua_guid_new(lua_State *L);
void ua_guid_tostring(lua_State *L, const UA_Guid *id);

/* NodeId */
int ua_nodeid_new(lua_State *L);
void ua_nodeid_tostring(lua_State *L, const UA_NodeId *id);

/* QualifiedName */
int ua_qualifiedname_new(lua_State *L);

/* Generic */
int ua_new(lua_State *L);
int ua_gc(lua_State *L);
int ua_index(lua_State *L);
int ua_newindex(lua_State *L);
int ua_tostring(lua_State *L);

/* Array */
int ua_array_index(lua_State *L);
int ua_array_len(lua_State *L);
int ua_array_newindex(lua_State *L);
int ua_array_append(lua_State *L);
int ua_array_remove(lua_State *L);
int ua_array_tostring(lua_State *L);
int ua_array_concat(lua_State *L);

/* Populate the Module */
int luaopen_open62541(lua_State *L);
