// This file is a part of uascript. License is MIT (see LICENSE file)

#include "libua.h"
#include "lauxlib.h"

static const struct luaL_Reg uascript_module [] = {
    {"typeof", ua_get_type},
    {"encodeBinary", ua_encodebinary},
    {"decodeBinary", ua_decodebinary},
    {"Array", ua_array_new},
    {"Server", ua_server_new},
    {"Client", ua_client_new},
    {"GetEndpoints", ua_client_getendpoints},
    {NULL, NULL} /* sentinel */
};

static void addNodeId(lua_State *L, UA_UInt32 identifier, const char *name) {
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = &UA_TYPES[UA_TYPES_NODEID];
    data->data = UA_NodeId_new();
    luaL_setmetatable(L, "open62541-data");
    *((UA_NodeId*)data->data) = UA_NODEID_NUMERIC(0, identifier);
    lua_setfield(L, -2, name);
}

static void addAttributeId(lua_State *L, int identifier, const char *name) {
    lua_pushnumber(L, identifier);
    lua_setfield(L, -2, name);
}

static int ua_indexerr(lua_State *L) {
    return luaL_error(L, "Type cannot be indexed");
}

int luaopen_ua(lua_State *L) {
    /* metatable for data types */
    luaL_newmetatable(L, "open62541-type");
    lua_pushcfunction(L, ua_type_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, ua_type_instantiate);
    lua_setfield(L, -2, "__call");
    lua_pushcfunction(L, ua_indexerr);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, ua_indexerr);
    lua_setfield(L, -2, "__newindex");
    lua_pop(L, 1);
    
    /* metatable for data */
    luaL_newmetatable(L, "open62541-data");
    lua_pushcfunction(L, ua_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, ua_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, ua_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, ua_pairs);
    lua_setfield(L, -2, "__pairs");
    lua_pushcfunction(L, ua_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    /* metatable for arrays */
    luaL_newmetatable(L, "open62541-array");
    lua_pushcfunction(L, ua_array_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, ua_array_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, ua_array_len);
    lua_setfield(L, -2, "__len");
    lua_pushcfunction(L, ua_array_pairs);
    lua_setfield(L, -2, "__pairs");
    lua_pushcfunction(L, ua_array_pairs);
    lua_setfield(L, -2, "__ipairs");
    lua_pushcfunction(L, ua_array_index);
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    /* metatable for the server */
    luaL_newmetatable(L, "open62541-server");
    lua_pushcfunction(L, ua_server_gc);
    lua_setfield(L, -2, "__gc");
    lua_newtable(L);
    lua_pushcfunction(L, ua_server_start);
    lua_setfield(L, -2, "start");
    lua_pushcfunction(L, ua_server_iterate);
    lua_setfield(L, -2, "iterate");
    lua_pushcfunction(L, ua_server_stop);
    lua_setfield(L, -2, "stop");
    lua_pushcfunction(L, ua_server_add_variablenode);
    lua_setfield(L, -2, "addVariableNode");
    lua_pushcfunction(L, ua_server_add_methodnode);
    lua_setfield(L, -2, "addMethodNode");
    lua_pushcfunction(L, ua_server_add_objectnode);
    lua_setfield(L, -2, "addObjectNode");
    lua_pushcfunction(L, ua_server_add_objecttypenode);
    lua_setfield(L, -2, "addObjectTypeNode");
    lua_pushcfunction(L, ua_server_add_referencetypenode);
    lua_setfield(L, -2, "addReferenceTypeNode");
    lua_pushcfunction(L, ua_server_add_reference);
    lua_setfield(L, -2, "addReference");
    lua_pushcfunction(L, ua_server_read);
    lua_setfield(L, -2, "read");
    lua_pushcfunction(L, ua_server_write);
    lua_setfield(L, -2, "write");
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    /* metatable for the client */
    luaL_newmetatable(L, "open62541-client");
    lua_pushcfunction(L, ua_client_gc);
    lua_setfield(L, -2, "__gc");
    lua_newtable(L);
    lua_pushcfunction(L, ua_client_connect);
    lua_setfield(L, -2, "connect");
    lua_pushcfunction(L, ua_client_disconnect);
    lua_setfield(L, -2, "disconnect");
    lua_pushcfunction(L, ua_client_service_browse);
    lua_setfield(L, -2, "browse");
    lua_pushcfunction(L, ua_client_service_browsenext);
    lua_setfield(L, -2, "browsenext");
    lua_pushcfunction(L, ua_client_service_read);
    lua_setfield(L, -2, "read");
    lua_pushcfunction(L, ua_client_service_write);
    lua_setfield(L, -2, "write");
    lua_pushcfunction(L, ua_client_service_call);
    lua_setfield(L, -2, "call");
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    /* create the module */
    luaL_newlib(L, uascript_module);

    /* add the data types */
    lua_newtable(L);
    for(size_t i = 0; i < UA_TYPES_COUNT; i++) {
        ua_type_push_typetable(L, &UA_TYPES[i]);
        lua_setfield(L, -2, UA_TYPES[i].typeName);
    }
    lua_setfield(L, -2, "types");

    /* add the nodeids */
    lua_newtable(L);
    addNodeId(L, 0, "Null");
    /* References */
    addNodeId(L, 31, "References");
    addNodeId(L, 32, "NonHierarchicalReferences");
    addNodeId(L, 33, "HierarchicalReferences");
    addNodeId(L, 35, "Organizes");
    addNodeId(L, 40, "HasTypeDefinition");
    addNodeId(L, 45, "HasSubtype");
    addNodeId(L, 46, "HasProperty");
    addNodeId(L, 47, "HasComponent");
    /* Object Types */
    addNodeId(L, 58, "BaseObjectType");
      addNodeId(L, 61, "FolderType");
    /* Objects */
    addNodeId(L, 84, "Root");
    addNodeId(L, 85, "Objects");
    addNodeId(L, 86, "Types");
      addNodeId(L, 90, "DataTypes");
      addNodeId(L, 88, "ObjectTypes");
      addNodeId(L, 91, "ReferenceTypes");
      addNodeId(L, 89, "VariableTypes");
    addNodeId(L, 87, "Views");
    lua_setfield(L, -2, "nodeIds");

    /* add the attribute ids */
    lua_newtable(L);
    addAttributeId(L, 1, "NodeId");
    addAttributeId(L, 2, "NodeClass");
    addAttributeId(L, 3, "BrowseName");
    addAttributeId(L, 4, "DisplayName");
    addAttributeId(L, 5, "Description");
    addAttributeId(L, 6, "WriteMask");
    addAttributeId(L, 7, "UserWriteMask");
    addAttributeId(L, 8, "IsAbstract");
    addAttributeId(L, 9, "Symmetric");
    addAttributeId(L, 10, "InverseName");
    addAttributeId(L, 11, "ContainsNoLoops");
    addAttributeId(L, 12, "EventNotifier");
    addAttributeId(L, 13, "Value");
    addAttributeId(L, 14, "DataType");
    addAttributeId(L, 15, "ValueRank");
    addAttributeId(L, 16, "ArrayDimensions");
    addAttributeId(L, 17, "AccessLevel");
    addAttributeId(L, 18, "UserAccessLevel");
    addAttributeId(L, 19, "MinimumSamplingInterval");
    addAttributeId(L, 20, "Historizing");
    addAttributeId(L, 21, "Executable");
    addAttributeId(L, 22, "UserExecutable");
    lua_setfield(L, -2, "attributeIds");

    return 1;
}
