// This file is a part of uascript. License is MIT (see LICENSE file)

#include "uascript.h"
#include "lualib.h"
#include "lauxlib.h"

struct ua_client {
    UA_Client *client;
};

int ua_client_new(lua_State *L) {
    struct ua_client *client = lua_newuserdata(L, sizeof(struct ua_client));
    client->client = UA_Client_new(UA_ClientConfig_standard, Logger_Stdout);
    luaL_setmetatable(L, "open62541-client");
    return 1;
}

int ua_client_gc(lua_State *L) {
    struct ua_client *client = luaL_checkudata (L, -1, "open62541-client");
    if(client)
        UA_Client_delete(client->client);
    return 0;
}

int ua_client_connect(lua_State *L) {
    struct ua_client *client = luaL_checkudata (L, 1, "open62541-client");
    if(!client)
        return luaL_error(L, "Not a client object");
    if(!lua_isstring(L, 2))
        return luaL_error(L, "Supply a connection string of the form opc.tcp://url:port");
    UA_StatusCode retval = UA_Client_connect(client->client, UA_ClientConnectionTCP,
                                             lua_tostring(L, 2));
    lua_pushinteger(L, retval);
    return 1;
}

int ua_client_getendpoints(lua_State *L) {
    if(!lua_isstring(L, 1))
        return luaL_error(L, "Supply a connection string of the form opc.tcp://url:port");
    UA_Client *client = UA_Client_new(UA_ClientConfig_standard, Logger_Stdout);
    size_t endpointsSize;
    UA_EndpointDescription *endpoints;
    UA_StatusCode retval = UA_Client_getEndpoints(client, UA_ClientConnectionTCP, lua_tostring(L, 1),
                                                  &endpointsSize, &endpoints);
    if(retval == UA_STATUSCODE_GOOD) {
        ua_array *array = lua_newuserdata(L, sizeof(ua_array));
        array->type = &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION];
        array->data = &array->local_data;
        array->length = &array->local_length;
        array->local_data = endpoints;
        array->local_length = endpointsSize;
        luaL_setmetatable(L, "open62541-array");
    } else {
        lua_pushnil(L);
    }
    lua_pushinteger(L, retval);
    UA_Client_disconnect(client);
    UA_Client_delete(client);
    return 2;
}

int ua_client_disconnect(lua_State *L) {
    struct ua_client *client = luaL_checkudata (L, 1, "open62541-client");
    if(!client)
        return luaL_error(L, "Not a client object");
    UA_StatusCode retval = UA_Client_disconnect(client->client);
    lua_pushinteger(L, retval);
    return 1;
}

static int
ua_client_service(lua_State *L, const UA_DataType *requestType, const char *operationsArray,
                  const UA_DataType *responseType, const char *resultsArray) {
    struct ua_client *client = luaL_checkudata (L, 1, "open62541-client");
    if(!client)
        return luaL_error(L, "The first argument must be a client object");

    ua_data *request = lua_newuserdata(L, sizeof(ua_data));
    request->data = UA_new(requestType);
    request->type = requestType;
    luaL_setmetatable(L, "open62541-data");

    ua_array *arr = ua_getarray(L, 2);
    size_t *operationsSize;
    void *operations;
    if(arr) {
        lua_pushstring(L, operationsArray);
        lua_pushvalue(L, 2);
        ua_newindex(L);
    } else {
        const UA_DataType *membertype;
        operations = ua_getmember(request->data, requestType, operationsArray,
                                  &membertype, &operationsSize);
        /* pointer a single operation into the request operations array */
        ua_data *data = ua_getdata(L, 2, membertype);
        if(!data)
            return luaL_error(L, "The second argument must be an (array of) operations");
        *operationsSize = 1;
        *(void**)operations = data->data;
    }
    
    ua_data *response = lua_newuserdata(L, sizeof(ua_data));
    response->data = UA_new(responseType);
    response->type = responseType;
    luaL_setmetatable(L, "open62541-data");
    __UA_Client_Service(client->client, request->data, requestType, response->data, responseType);

    if(!arr) {
        /* remove the single operation from the request */
        *operationsSize = 0;
        *(void**)operations = NULL;
    }

    /* ua_data *resp_data = lua_newuserdata(L, sizeof(ua_data)); */
    /* resp_data->data = response; */
    /* resp_data->type = responseType; */
    /* luaL_setmetatable(L, "open62541-data"); */
    return 1;
}

int ua_client_service_browse(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_BROWSEREQUEST], "nodesToBrowse",
                             &UA_TYPES[UA_TYPES_BROWSERESPONSE], "results");
}

int ua_client_service_browsenext(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST], "continuationPoints",
                             &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE], "results");
}

int ua_client_service_read(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_READREQUEST], "nodesToRead",
                             &UA_TYPES[UA_TYPES_READRESPONSE], "results");
}

int ua_client_service_write(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_WRITEREQUEST], "nodesToWrite",
                             &UA_TYPES[UA_TYPES_WRITERESPONSE], "results");
}

int ua_client_service_call(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_CALLREQUEST], "methodsToCall",
                             &UA_TYPES[UA_TYPES_CALLRESPONSE], "results");
}
