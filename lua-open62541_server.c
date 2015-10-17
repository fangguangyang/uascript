#ifndef _WIN32
# include <pthread.h>
#else
# include <windows.h>
# include <process.h>
#endif
#include "lua-open62541.h"

struct ua_background_server {
    UA_Boolean running;
    UA_Boolean gc;
    UA_Server *server;
#ifndef _WIN32
    pthread_t thread;
#else
    uintptr_t thread;
#endif
};

static void * run_server(struct ua_background_server *s) {
    UA_Server_run(s->server, 1, &s->running);
    return NULL;
}

int ua_server_new(lua_State *L) {
    int port = 16664;
    if(lua_isnumber(L, 1))
        port = lua_tonumber(L, 1);
    struct ua_background_server *server = lua_newuserdata(L, sizeof(struct ua_background_server));
    server->running = UA_FALSE;
    server->gc = UA_TRUE;
    server->server = UA_Server_new(UA_ServerConfig_standard);
#ifndef _WIN32
    server->thread = pthread_self();
#else
    server->thread = 0;
#endif
    UA_Server_setLogger(server->server, Logger_Stdout);
    UA_ServerNetworkLayer *nl;
    nl = ServerNetworkLayerTCP_new(UA_ConnectionConfig_standard, port);
    UA_Server_addNetworkLayer(server->server, nl);
    luaL_setmetatable(L, "open62541-server");
    return 1;
}

int ua_server_gc(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, -1, "open62541-server");
    if(server->gc)
        UA_Server_delete(server->server);
    return 0;
}

int ua_server_start(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, -1, "open62541-server");
#ifndef _WIN32
    if(!pthread_equal(pthread_self(), server->thread))
        return luaL_error(L, "The server is already running");
    server->running = UA_TRUE;
    pthread_create(&server->thread, NULL, (void*(*)(void*))run_server, (void*)server);
#else
    if(server->thread)
        return luaL_error(L, "The server is already running");
    server->running = UA_TRUE;
    server->thread = _beginthread((void(*)(void*))run_server, 0, (void*)server);
#endif
    return 0;
}

int ua_server_stop(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, -1, "open62541-server");
    if(pthread_equal(pthread_self(), server->thread))
        return luaL_error(L, "The server is not running");
    server->running = UA_FALSE;
#ifndef _WIN32
    pthread_join(server->thread, NULL);
    server->thread = pthread_self();
#else
    server->thread = 0;
#endif
    return 0;
}

int ua_server_add_variablenode(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    if(!server)
        return luaL_error(L, "function must be called on a server");
    ua_data *requestedNewNodeId = ua_getdata(L, 2);
    if(!requestedNewNodeId || requestedNewNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "1st argument (requestednewnodeid) is not of nodeid type");
    ua_data *parentNodeId = ua_getdata(L, 3);
    if(!parentNodeId || parentNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "2nd argument (parentnodeid) is not of nodeid type");
    ua_data *referenceTypeId = ua_getdata(L, 4);
    if(!referenceTypeId || referenceTypeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "3th argument (referenceTypeId) is not of nodeid type");
    ua_data *browseName = ua_getdata(L, 5);
    if(!browseName || browseName->type != &UA_TYPES[UA_TYPES_QUALIFIEDNAME])
        return luaL_error(L, "4th argument (browsename) is not of qualifiedname type");
    ua_data *typeDefinition = ua_getdata(L, 6);
    if(!typeDefinition || typeDefinition->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "5th argument (typeDefinition) is not of nodeid type");
    ua_data *attr = ua_getdata(L, 7);
    if(!attr || attr->type != &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES])
        return luaL_error(L, "6th argument (variableattributes) is not of variableattributes type");
    UA_NodeId result;
    UA_StatusCode retval =
        UA_Server_addVariableNode(server->server, *(UA_NodeId*)requestedNewNodeId->data,
                                  *(UA_NodeId*)parentNodeId->data, *(UA_NodeId*)referenceTypeId->data,
                                  *(UA_QualifiedName*)browseName->data, *(UA_NodeId*)typeDefinition->data,
                                  *(UA_VariableAttributes*)attr->data, &result);
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
    ua_data *requestedNewNodeId = ua_getdata(L, 2);
    if(!requestedNewNodeId || requestedNewNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "1st argument (requestednewnodeid) is not of nodeid type");
    ua_data *parentNodeId = ua_getdata(L, 3);
    if(!parentNodeId || parentNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "2nd argument (parentnodeid) is not of nodeid type");
    ua_data *referenceTypeId = ua_getdata(L, 4);
    if(!referenceTypeId || referenceTypeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "3th argument (referenceTypeId) is not of nodeid type");
    ua_data *browseName = ua_getdata(L, 5);
    if(!browseName || browseName->type != &UA_TYPES[UA_TYPES_QUALIFIEDNAME])
        return luaL_error(L, "4th argument (browsename) is not of qualifiedname type");
    ua_data *typeDefinition = ua_getdata(L, 6);
    if(!typeDefinition || typeDefinition->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "5th argument (typeDefinition) is not of nodeid type");
    ua_data *attr = ua_getdata(L, 7);
    if(!attr || attr->type != &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES])
        return luaL_error(L, "6th argument is not of objectattributes type");
    UA_NodeId result;
    UA_StatusCode retval =
        UA_Server_addObjectNode(server->server, *(UA_NodeId*)requestedNewNodeId->data,
                                *(UA_NodeId*)parentNodeId->data, *(UA_NodeId*)referenceTypeId->data,
                                *(UA_QualifiedName*)browseName->data, *(UA_NodeId*)typeDefinition->data,
                                *(UA_ObjectAttributes*)attr->data, &result);
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
    ua_data *requestedNewNodeId = ua_getdata(L, 2);
    if(!requestedNewNodeId || !requestedNewNodeId || requestedNewNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "1st argument (requestednewnodeid) is not of nodeid type");
    ua_data *parentNodeId = ua_getdata(L, 3);
    if(!parentNodeId || parentNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "2nd argument (parentnodeid) is not of nodeid type");
    ua_data *referenceTypeId = ua_getdata(L, 4);
    if(!referenceTypeId || referenceTypeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "3th argument (referenceTypeId) is not of nodeid type");
    ua_data *browseName = ua_getdata(L, 5);
    if(!browseName || browseName->type != &UA_TYPES[UA_TYPES_QUALIFIEDNAME])
        return luaL_error(L, "4th argument (browsename) is not of qualifiedname type");
    ua_data *attr = ua_getdata(L, 6);
    if(!attr || attr->type != &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES])
        return luaL_error(L, "5th argument is not of objecttypeattributes type");
    UA_NodeId result;
    UA_StatusCode retval =
        UA_Server_addObjectTypeNode(server->server, *(UA_NodeId*)requestedNewNodeId->data,
                                    *(UA_NodeId*)parentNodeId->data, *(UA_NodeId*)referenceTypeId->data,
                                    *(UA_QualifiedName*)browseName->data,
                                    *(UA_ObjectTypeAttributes*)attr->data, &result);
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
    ua_data *requestedNewNodeId = ua_getdata(L, 2);
    if(!requestedNewNodeId || requestedNewNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "1st argument (requestednewnodeid) is not of nodeid type");
    ua_data *parentNodeId = ua_getdata(L, 3);
    if(!parentNodeId || parentNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "2nd argument (parentnodeid) is not of nodeid type");
    ua_data *referenceTypeId = ua_getdata(L, 4);
    if(!referenceTypeId || referenceTypeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "3th argument (referenceTypeId) is not of nodeid type");
    ua_data *browseName = ua_getdata(L, 5);
    if(!browseName || browseName->type != &UA_TYPES[UA_TYPES_QUALIFIEDNAME])
        return luaL_error(L, "4th argument (browsename) is not of qualifiedname type");
    ua_data *attr = ua_getdata(L, 6);
    if(!attr || attr->type != &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES])
        return luaL_error(L, "5th argument is not of referencetypeattributes type");
    UA_NodeId result;
    UA_StatusCode retval =
        UA_Server_addReferenceTypeNode(server->server, *(UA_NodeId*)requestedNewNodeId->data,
                                       *(UA_NodeId*)parentNodeId->data, *(UA_NodeId*)referenceTypeId->data,
                                       *(UA_QualifiedName*)browseName->data,
                                       *(UA_ReferenceTypeAttributes*)attr->data, &result);
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
    UA_Server *server;
    UA_ByteString function_dump;
};

static int dump_write(lua_State* L, unsigned char* str, size_t len, struct luaL_Buffer *buf) {
    luaL_addlstring(buf, str, len);
    return 0;
}

static UA_StatusCode
ua_server_methodcallback(const UA_NodeId objectId, const UA_Variant *input,
                         UA_Variant *output, void *handle) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    luaopen_open62541(L);
    struct callbackdata *data = handle;
    luaL_loadbuffer(L, data->function_dump.data, data->function_dump.length, "callback function");

    struct ua_background_server *server = lua_newuserdata(L, sizeof(struct ua_background_server));
    server->running = UA_TRUE;
    server->gc = UA_FALSE;
    server->server = data->server;
#ifndef _WIN32
    server->thread = pthread_self();
#else
    server->thread = 0;
#endif
    luaL_setmetatable(L, "open62541-server");

    ua_data *id = lua_newuserdata(L, sizeof(ua_data));
    id->data = UA_NodeId_new();
    UA_NodeId_copy(&objectId, id->data);
    id->type = &UA_TYPES[UA_TYPES_NODEID];
    luaL_setmetatable(L, "open62541-builtin");

    if(input) {
        ua_data *i = lua_newuserdata(L, sizeof(ua_data));
        i->data = UA_String_new();
        UA_String_copy(input->data, i->data);
        i->type = &UA_TYPES[UA_TYPES_STRING];
        /* i->data = UA_Variant_new(); */
        /* UA_Variant_copy(input, i->data); */
        /* i->type = &UA_TYPES[UA_TYPES_VARIANT]; */
        luaL_setmetatable(L, "open62541-builtin");
    } else
        lua_pushnil(L);

    int res = lua_pcall(L, 3, 2, 0); // 0 -> success
    if(res) {
        printf("error in the callback: %s", lua_tostring(L, -1));
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    UA_StatusCode retval = lua_tonumber(L, -2);
    ua_data *out = ua_getdata(L, -1);
    if(!output || !out)
        ;
    else if(out->type == &UA_TYPES[UA_TYPES_VARIANT])
        UA_Variant_copy(output, out->data);
    else
        UA_Variant_setScalarCopy(output, out->data, out->type);
    lua_close(L);
    return retval;
}

int ua_server_add_methodnode(lua_State *L) {
    struct ua_background_server *server = luaL_checkudata (L, 1, "open62541-server");
    ua_data *requestedNewNodeId = ua_getdata(L, 2);
    if(requestedNewNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "1st argument (requestednewnodeid) is not of nodeid type");
    ua_data *parentNodeId = ua_getdata(L, 3);
    if(parentNodeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "2nd argument (parentnodeid) is not of nodeid type");
    ua_data *referenceTypeId = ua_getdata(L, 4);
    if(referenceTypeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "3th argument (referenceTypeId) is not of nodeid type");
    ua_data *browseName = ua_getdata(L, 5);
    if(browseName->type != &UA_TYPES[UA_TYPES_QUALIFIEDNAME])
        return luaL_error(L, "4th argument (browsename) is not of qualifiedname type");
    ua_data *attr = ua_getdata(L, 6);
    if(attr->type != &UA_TYPES[UA_TYPES_METHODATTRIBUTES])
        return luaL_error(L, "5th argument (methodattributes) is not of methodattributes type");
    if(!lua_isfunction(L, 7))
        return luaL_error(L, "6th argument (method) is not of function type");
    ua_array *input = ua_getarray(L, 8);
    if(!input || (input->type && input->type != &UA_TYPES[UA_TYPES_ARGUMENT]))
        return luaL_error(L, "7th argument (inputarguments) is not an array of arguments");
    ua_array *output = ua_getarray(L, 9);
    if(!output || (output->type && output->type != &UA_TYPES[UA_TYPES_ARGUMENT]))
        return luaL_error(L, "8th argument (outputarguments) is not an array of arguments");

    /* dump the function */
    luaL_Buffer buf;
    luaL_buffinit(L, &buf);
    lua_pushvalue(L, 7);
    lua_dump(L, (lua_Writer)dump_write, &buf);
    luaL_pushresult(&buf);
    size_t dumpsize;
    const char *dbuffer = lua_tolstring(L, -1, &dumpsize);

    struct callbackdata *cbdata = malloc(sizeof(struct callbackdata));
    cbdata->server = server->server;
    UA_ByteString_newMembers(&cbdata->function_dump, dumpsize);
    memcpy(cbdata->function_dump.data, dbuffer, dumpsize);

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
    ua_data *sourceId = ua_getdata(L, 2);
    if(!sourceId || sourceId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "1st argument (sourceid) is not of nodeid type");
    ua_data *refTypeId = ua_getdata(L, 3);
    if(!refTypeId || refTypeId->type != &UA_TYPES[UA_TYPES_NODEID])
        return luaL_error(L, "2nd argument (reftypeid) is not of nodeid type");
    ua_data *targetId = ua_getdata(L, 4);
    if(!targetId || (targetId->type != &UA_TYPES[UA_TYPES_EXPANDEDNODEID] &&
                     targetId->type != &UA_TYPES[UA_TYPES_NODEID]))
        return luaL_error(L, "3rd argument (targetid) is not of nodeid or expandednodeid type");
    ua_data *isForward = ua_getdata(L, 5);
    if(!isForward || isForward->type != &UA_TYPES[UA_TYPES_BOOLEAN])
        return luaL_error(L, "4th argument (isforward) is not of boolean type");

    UA_ExpandedNodeId target;
    UA_ExpandedNodeId_init(&target);
    if(targetId->type == &UA_TYPES[UA_TYPES_EXPANDEDNODEID])
        target = *(UA_ExpandedNodeId*)targetId->data;
    else
        target.nodeId = *(UA_NodeId*)targetId->data;

    UA_StatusCode retval =
        UA_Server_addReference(server->server, *(UA_NodeId*)sourceId->data,
                               *(UA_NodeId*)refTypeId->data,
                               target, *(UA_Boolean*)isForward->data);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = &UA_TYPES[UA_TYPES_STATUSCODE];
    data->data = UA_StatusCode_new();
    *(UA_StatusCode*)data->data = retval;
    luaL_setmetatable(L, "open62541-data");
    return 1;
}
