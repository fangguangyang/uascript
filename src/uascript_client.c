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
    UA_StatusCode retval = UA_Client_connect(client->client, ClientNetworkLayerTCP_connect,
                                             lua_tostring(L, 2));
    lua_pushinteger(L, retval);
    return 1;
}

int ua_client_disconnect(lua_State *L) {
    struct ua_client *client = luaL_checkudata (L, 1, "open62541-client");
    if(!client)
        return luaL_error(L, "Not a client object");
    UA_StatusCode retval = UA_Client_disconnect(client->client);
    lua_pushinteger(L, retval);
    return 1;
}

static int ua_client_service(lua_State *L, const UA_DataType *requestType, const UA_DataType *responseType) {
    struct ua_client *client = luaL_checkudata (L, 1, "open62541-client");
    if(!client)
        return luaL_error(L, "Not a client object");

    ua_data *data = luaL_testudata(L, 2 , "open62541-data");
    if(!data || data->type != requestType)
        return luaL_error(L, "The request type does not match");

    UA_BrowseResponse *response = UA_new(responseType);
    __UA_Client_Service(client->client, data->data, requestType, response, responseType);
    ua_data *resp_data = lua_newuserdata(L, sizeof(ua_data));
    resp_data->data = response;
    resp_data->type = responseType;
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_client_service_browse(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_BROWSEREQUEST], &UA_TYPES[UA_TYPES_BROWSERESPONSE]);
}

int ua_client_service_browsenext(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST], &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]);
}

int ua_client_service_read(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_READREQUEST], &UA_TYPES[UA_TYPES_READRESPONSE]);
}

int ua_client_service_write(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_WRITEREQUEST], &UA_TYPES[UA_TYPES_WRITERESPONSE]);
}

int ua_client_service_call(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_CALLREQUEST], &UA_TYPES[UA_TYPES_CALLRESPONSE]);
}
