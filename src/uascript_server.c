// This file is a part of uascript. License is MIT (see LICENSE file)

#include "uascript.h"
#include "lualib.h"
#include "lauxlib.h"

struct ua_background_server {
    UA_ServerNetworkLayer nl;
    UA_Server *server;
};

int ua_server_new(lua_State *L) {
    if(!lua_isnumber(L, 1))
        return luaL_error(L, "The 1st argument must be the server port");
    int port = lua_tonumber(L, 1);
    struct ua_background_server *server = lua_newuserdata(L, sizeof(struct ua_background_server));
    server->nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, port);
    UA_ServerConfig config = UA_ServerConfig_standard;
    config.logger = Logger_Stdout;
    config.networkLayers = &server->nl;
    config.networkLayersSize = 1;
    server->server = UA_Server_new(config);
    luaL_setmetatable(L, "open62541-server");
    return 1;
}

int ua_server_gc(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, -1, "open62541-server");
    server->nl.deleteMembers(&server->nl);
    UA_Server_delete(server->server);
    return 0;
}

int ua_server_start(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, -1, "open62541-server");
    lua_pushinteger(L, UA_Server_run_startup(server->server));
    return 1;
}

int ua_server_iterate(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, -1, "open62541-server");
    lua_pushinteger(L, UA_Server_run_iterate(server->server));
    return 1;
}

int ua_server_stop(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, -1, "open62541-server");
    lua_pushinteger(L, UA_Server_run_shutdown(server->server));
    return 1;
}

int ua_server_add_variablenode(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    if(!server)
        return luaL_error(L, "function must be called on a server");
    ua_data *requestedNewNodeId = ua_getdata(L, 2, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *parentNodeId = ua_getdata(L, 3, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *referenceTypeId = ua_getdata(L, 4, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *browseName = ua_getdata(L, 5, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    ua_data *typeDefinition = ua_getdata(L, 6, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *attr = ua_getdata(L, 7, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]);
    UA_NodeId result;
    UA_StatusCode retval;
    retval = UA_Server_addVariableNode(server->server, *(UA_NodeId*)requestedNewNodeId->data,
                                       *(UA_NodeId*)parentNodeId->data, *(UA_NodeId*)referenceTypeId->data,
                                       *(UA_QualifiedName*)browseName->data, *(UA_NodeId*)typeDefinition->data,
                                       *(UA_VariableAttributes*)attr->data, NULL, &result);
    if(retval != UA_STATUSCODE_GOOD)
        return luaL_error(L, "Statuscode is %f", retval);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = &UA_TYPES[UA_TYPES_NODEID];
    data->data = UA_NodeId_new();
    *(UA_NodeId*)data->data = result;
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_server_add_objectnode(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    if(!server)
        return luaL_error(L, "function must be called on a server");
    ua_data *requestedNewNodeId = ua_getdata(L, 2, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *parentNodeId = ua_getdata(L, 3, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *referenceTypeId = ua_getdata(L, 4, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *browseName = ua_getdata(L, 5, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    ua_data *typeDefinition = ua_getdata(L, 6, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *attr = ua_getdata(L, 7, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]);
    UA_NodeId result;
    UA_StatusCode retval;
    retval = UA_Server_addObjectNode(server->server, *(UA_NodeId*)requestedNewNodeId->data,
                                     *(UA_NodeId*)parentNodeId->data, *(UA_NodeId*)referenceTypeId->data,
                                     *(UA_QualifiedName*)browseName->data, *(UA_NodeId*)typeDefinition->data,
                                     *(UA_ObjectAttributes*)attr->data, NULL, &result);
    if(retval != UA_STATUSCODE_GOOD)
        return luaL_error(L, "Statuscode is %f", retval);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = &UA_TYPES[UA_TYPES_NODEID];
    data->data = UA_NodeId_new();
    *(UA_NodeId*)data->data = result;
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_server_add_objecttypenode(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    if(!server)
        return luaL_error(L, "function must be called on a server");
    ua_data *requestedNewNodeId = ua_getdata(L, 2, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *parentNodeId = ua_getdata(L, 3, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *referenceTypeId = ua_getdata(L, 4, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *browseName = ua_getdata(L, 5, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    ua_data *attr = ua_getdata(L, 6, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]);
    UA_NodeId result;
    UA_StatusCode retval;
    retval = UA_Server_addObjectTypeNode(server->server, *(UA_NodeId*)requestedNewNodeId->data,
                                         *(UA_NodeId*)parentNodeId->data, *(UA_NodeId*)referenceTypeId->data,
                                         *(UA_QualifiedName*)browseName->data,
                                         *(UA_ObjectTypeAttributes*)attr->data, NULL, &result);
    if(retval != UA_STATUSCODE_GOOD)
        return luaL_error(L, "Statuscode is %f", retval);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = &UA_TYPES[UA_TYPES_NODEID];
    data->data = UA_NodeId_new();
    *(UA_NodeId*)data->data = result;
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_server_add_referencetypenode(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    if(!server)
        return luaL_error(L, "function must be called on a server");
    ua_data *requestedNewNodeId = ua_getdata(L, 2, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *parentNodeId = ua_getdata(L, 3, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *referenceTypeId = ua_getdata(L, 4, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *browseName = ua_getdata(L, 5, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    ua_data *attr = ua_getdata(L, 6, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]);
    UA_NodeId result;
    UA_StatusCode retval;
    retval = UA_Server_addReferenceTypeNode(server->server, *(UA_NodeId*)requestedNewNodeId->data,
                                            *(UA_NodeId*)parentNodeId->data, *(UA_NodeId*)referenceTypeId->data,
                                            *(UA_QualifiedName*)browseName->data,
                                            *(UA_ReferenceTypeAttributes*)attr->data, NULL, &result);
    if(retval != UA_STATUSCODE_GOOD)
        return luaL_error(L, "Statuscode is %f", retval);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = &UA_TYPES[UA_TYPES_NODEID];
    data->data = UA_NodeId_new();
    *(UA_NodeId*)data->data = result;
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

struct callbackdata {
    lua_State *L;
    void* functionindex;
};

static UA_StatusCode
ua_server_methodcallback(void *methodHandle, const UA_NodeId objectId,
                         size_t inputSize, const UA_Variant *input,
                         size_t outputSize, UA_Variant *output) {
    struct callbackdata *data = methodHandle;
    lua_State *L = lua_newthread(data->L);
    /* get the function */
    lua_pushlightuserdata(L, data->functionindex);
    lua_gettable(L, LUA_REGISTRYINDEX);

    ua_data *id = lua_newuserdata(L, sizeof(ua_data));
    id->data = UA_NodeId_new();
    UA_NodeId_copy(&objectId, id->data);
    id->type = &UA_TYPES[UA_TYPES_NODEID];
    luaL_setmetatable(L, "open62541-data");

    for(size_t i = 0; i < inputSize; i++) {
        ua_data *arg = lua_newuserdata(L, sizeof(ua_data));
        arg->data = UA_Variant_new();
        UA_Variant_copy(&input[i], arg->data);
        arg->type = &UA_TYPES[UA_TYPES_VARIANT];
        luaL_setmetatable(L, "open62541-data");
    }

    int res = lua_pcall(L, inputSize+1, outputSize, 0); // the first output is the statuscode
    if(res) {
        printf("error in the callback: %s\n", lua_tostring(L, -1));
        return UA_STATUSCODE_BADINTERNALERROR;
    }

    for(size_t j = 0; j < outputSize; j++) {
        ua_data *out = ua_getdata(L, -outputSize-1, NULL);
        if(out) {
            if(out->type == &UA_TYPES[UA_TYPES_VARIANT])
                UA_Variant_copy(out->data, &output[j]);
            else
                UA_Variant_setScalarCopy(&output[j], out->data, out->type);
            continue;
        }
        ua_array *outarray = luaL_testudata(L, -outputSize-1, "open62541-array");
        if(outarray) {
            UA_Variant_setArrayCopy(&output[j], *outarray->data, *outarray->length, outarray->type);
            continue;
        }
        return UA_STATUSCODE_BADINTERNALERROR;
    }

    lua_pop(data->L, 1); // pop the new thread from the original state
    return UA_STATUSCODE_GOOD;
}

int ua_server_add_methodnode(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    ua_data *requestedNewNodeId = ua_getdata(L, 2, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *parentNodeId = ua_getdata(L, 3, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *referenceTypeId = ua_getdata(L, 4, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *browseName = ua_getdata(L, 5, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
    ua_data *attr = ua_getdata(L, 6, &UA_TYPES[UA_TYPES_METHODATTRIBUTES]);
    if(!lua_isfunction(L, 7))
        return luaL_error(L, "6th argument (method) is not of function type");
    ua_array *input = luaL_checkudata(L, 8, "open62541-array");
    if(!input || (input->type && input->type != &UA_TYPES[UA_TYPES_ARGUMENT]))
        return luaL_error(L, "7th argument (inputarguments) is not an array of arguments");
    ua_array *output = luaL_checkudata(L, 9, "open62541-array");
    if(!output || (output->type && output->type != &UA_TYPES[UA_TYPES_ARGUMENT]))
        return luaL_error(L, "8th argument (outputarguments) is not an array of arguments");

    struct callbackdata *cbdata = malloc(sizeof(struct callbackdata));

    /* put the function in the registry */
    lua_pushlightuserdata(L, cbdata);
    lua_pushvalue(L, 7);
    lua_settable(L, LUA_REGISTRYINDEX);
    cbdata->L = L;
    cbdata->functionindex = (void*)cbdata;
    UA_NodeId result;
    UA_StatusCode retval = UA_Server_addMethodNode(server->server, *(UA_NodeId*)requestedNewNodeId->data,
                                                   *(UA_NodeId*)parentNodeId->data,
                                                   *(UA_NodeId*)referenceTypeId->data,
                                                   *(UA_QualifiedName*)browseName->data,
                                                   *(UA_MethodAttributes*)attr->data,
                                                   ua_server_methodcallback, cbdata,
                                                   *input->length, *input->data,
                                                   *output->length, *output->data, &result);

    if(retval != UA_STATUSCODE_GOOD)
        return luaL_error(L, "Statuscode is %f", retval);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = &UA_TYPES[UA_TYPES_NODEID];
    data->data = UA_NodeId_new();
    *(UA_NodeId*)data->data = result;
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_server_add_reference(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    if(!server)
        return luaL_error(L, "function must be called on a server");
    ua_data *sourceId = ua_getdata(L, 2, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *refTypeId = ua_getdata(L, 3, &UA_TYPES[UA_TYPES_NODEID]);
    ua_data *targetId = ua_getdata(L, 4, NULL);
    if(!targetId || (targetId->type != &UA_TYPES[UA_TYPES_EXPANDEDNODEID] &&
                     targetId->type != &UA_TYPES[UA_TYPES_NODEID]))
        return luaL_error(L, "3rd argument (targetid) is not of nodeid or expandednodeid type");
    ua_data *isForward = ua_getdata(L, 5, &UA_TYPES[UA_TYPES_BOOLEAN]);

    UA_ExpandedNodeId target;
    UA_ExpandedNodeId_init(&target);
    if(targetId->type == &UA_TYPES[UA_TYPES_EXPANDEDNODEID])
        target = *(UA_ExpandedNodeId*)targetId->data;
    else
        target.nodeId = *(UA_NodeId*)targetId->data;

    UA_StatusCode retval = UA_Server_addReference(server->server, *(UA_NodeId*)sourceId->data,
                                                  *(UA_NodeId*)refTypeId->data,
                                                  target, *(UA_Boolean*)isForward->data);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = &UA_TYPES[UA_TYPES_STATUSCODE];
    data->data = UA_StatusCode_new();
    *(UA_StatusCode*)data->data = retval;
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_server_write(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    if(!server)
        return luaL_error(L, "function must be called on a server");

    ua_data *sourceId = ua_getdata(L, 2, &UA_TYPES[UA_TYPES_NODEID]);
    if(!lua_isnumber(L, 3))
        return luaL_error(L, "2nd argument (attributeid) is not a number");
    lua_Number attrId = lua_tonumber(L, 3);

    ua_data *value = ua_getdata(L, 4, NULL);

    UA_StatusCode retval;
    if(attrId != UA_ATTRIBUTEID_VALUE) {
        retval = __UA_Server_write(server->server, (UA_NodeId*)sourceId->data,
                                   (UA_AttributeId)attrId, value->type, value->data);
    } else {
        UA_Variant v;
        UA_Variant_init(&v);
        UA_Variant_setScalarCopy(&v, value->data, value->type);
        retval = __UA_Server_write(server->server, (UA_NodeId*)sourceId->data,
                                   (UA_AttributeId)attrId, &UA_TYPES[UA_TYPES_VARIANT], &v);
    }
    UA_WriteValue wv;
    UA_WriteValue_init(&wv);
    UA_Variant_setScalar(&wv.value.value, value->data, value->type);
    lua_pushnumber(L, retval);
    return 1;
}

int ua_server_read(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    if(!server)
        return luaL_error(L, "function must be called on a server");

    ua_data *sourceId = ua_getdata(L, 2, &UA_TYPES[UA_TYPES_NODEID]);

    if(!lua_isnumber(L, 3))
        return luaL_error(L, "2nd argument (attributeid) is not a number");
    lua_Number attrId = lua_tonumber(L, 3);

    const UA_DataType *type;
    switch((int)attrId) {
    case UA_ATTRIBUTEID_NODEID:
        type = &UA_TYPES[UA_TYPES_NODEID];
        break;
    case UA_ATTRIBUTEID_NODECLASS:
        type = &UA_TYPES[UA_TYPES_NODECLASS];
        break;
    case UA_ATTRIBUTEID_BROWSENAME:
        type = &UA_TYPES[UA_TYPES_QUALIFIEDNAME];
        break;
    case UA_ATTRIBUTEID_DISPLAYNAME:
        type = &UA_TYPES[UA_TYPES_LOCALIZEDTEXT];
        break;
    case UA_ATTRIBUTEID_DESCRIPTION:
        type = &UA_TYPES[UA_TYPES_LOCALIZEDTEXT];
        break;
    case UA_ATTRIBUTEID_WRITEMASK:
        type = &UA_TYPES[UA_TYPES_UINT32];
        break;
    case UA_ATTRIBUTEID_USERWRITEMASK:
        type = &UA_TYPES[UA_TYPES_UINT32];
        break;
    case UA_ATTRIBUTEID_ISABSTRACT:
        type = &UA_TYPES[UA_TYPES_BOOLEAN];
        break;
    case UA_ATTRIBUTEID_SYMMETRIC:
        type = &UA_TYPES[UA_TYPES_BOOLEAN];
        break;
    case UA_ATTRIBUTEID_INVERSENAME:
        type = &UA_TYPES[UA_TYPES_LOCALIZEDTEXT];
        break;
    case UA_ATTRIBUTEID_CONTAINSNOLOOPS:
        type = &UA_TYPES[UA_TYPES_BOOLEAN];
        break;
    case UA_ATTRIBUTEID_EVENTNOTIFIER:
        type = &UA_TYPES[UA_TYPES_BYTE];
        break;
    case UA_ATTRIBUTEID_VALUE:
        type = &UA_TYPES[UA_TYPES_VARIANT];
        break;
    case UA_ATTRIBUTEID_DATATYPE:
        type = &UA_TYPES[UA_TYPES_NODEID];
        break;
    case UA_ATTRIBUTEID_VALUERANK:
        type = &UA_TYPES[UA_TYPES_INT32];
        break;
    case UA_ATTRIBUTEID_ARRAYDIMENSIONS:
        return luaL_error(L, "Returning arrays is not supported");
    case UA_ATTRIBUTEID_ACCESSLEVEL:
        type = &UA_TYPES[UA_TYPES_UINT32];
        break;
    case UA_ATTRIBUTEID_USERACCESSLEVEL:
        type = &UA_TYPES[UA_TYPES_UINT32];
        break;
    case UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL:
        type = &UA_TYPES[UA_TYPES_DOUBLE];
        break;
    case UA_ATTRIBUTEID_HISTORIZING:
    case UA_ATTRIBUTEID_EXECUTABLE:
    case UA_ATTRIBUTEID_USEREXECUTABLE:
        type = &UA_TYPES[UA_TYPES_BOOLEAN];
        break;
    default:
        return luaL_error(L, "Unknown attribute");
    }
    void *v = UA_new(type);
    UA_StatusCode retval = __UA_Server_read(server->server, sourceId->data, attrId, v);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_delete(v, type);
        lua_pushnil(L);
        lua_pushinteger(L, retval);
        return 2;
    }
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = type;
    data->data = v;
    luaL_setmetatable(L, "open62541-data");
    return 1;
}
