// This file is a part of uascript. License is MIT (see LICENSE file)

#include "libua.h"
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
ua_client_service(lua_State *L, const UA_DataType *requestType, const char **inputs,
                  const UA_DataType *responseType, const char **outputs) {
    struct ua_client *client = luaL_checkudata (L, 1, "open62541-client");
    if(!client)
        return luaL_error(L, "The first argument must be a client object");

    /* fill up the arguments before adding new elements to the stack */
    for(size_t i = 0; ; i++ ) {
        if(!inputs[i])
            break;
        lua_pushnil(L);
    }

    ua_data *request = lua_newuserdata(L, sizeof(ua_data));
    request->data = UA_new(requestType);
    request->type = requestType;
    luaL_setmetatable(L, "open62541-data");

    for(size_t i = 0; ; i++ ) {
        const char *input = inputs[i];
        if(!input)
            break;
        if(lua_isnil(L, i+2))
            continue;
        /* get the member in the request */
        size_t *inputlen;
        const UA_DataType *inputType;
        void *inputdata = ua_getmember(request->data, requestType, input, &inputType, &inputlen);
        if(!inputlen) {
            ua_data *data = ua_getdata(L, i+2, inputType);
            UA_copy(data->data, inputdata, inputType);
        } else {
            ua_array *array = ua_getarray(L, i+2);
            if(!array || array->type != inputType)
                return luaL_error(L, "Argument %d must be an array of type %s.",
                                  i+1, inputType->typeName);
            UA_Array_copy(*array->data, *array->length, (void**)inputdata, inputType);
            *inputlen = *array->length;
        }
    }

    ua_data *response = lua_newuserdata(L, sizeof(ua_data));
    response->data = UA_new(responseType);
    response->type = responseType;
    luaL_setmetatable(L, "open62541-data");
    __UA_Client_Service(client->client, request->data, requestType, response->data, responseType);

    int out = 0;
    for(size_t i = 0; ; i++) {
        const char *output = outputs[i];
        if(!output)
            break;
        /* get the member in the response */
        size_t *outputlen;
        const UA_DataType *outputType;
        void *outputdata = ua_getmember(response->data, responseType, output, &outputType, &outputlen);
        if(!outputlen) {
            ua_data *data = lua_newuserdata(L, sizeof(ua_data));
            data->data = UA_new(outputType);
            data->type = outputType;
            luaL_setmetatable(L, "open62541-data");
            UA_copy(outputdata, data->data, outputType);
        } else {
            ua_array *array = lua_newuserdata(L, sizeof(ua_array));
            array->type = outputType;
            array->data = &array->local_data;
            array->length = &array->local_length;
            array->local_length = *outputlen;
            luaL_setmetatable(L, "open62541-array");
            UA_Array_copy(*(void**)outputdata, *outputlen, &array->local_data, outputType);
        }
        out++;
    }
    return out;
}

static const char *service_browse_input[] = {"nodesToBrowse", "requestedMaxReferencesPerNode", "view", NULL};
static const char *service_browse_output[] = {"results", "responseHeader", "diagnosticInfos", NULL};
int ua_client_service_browse(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_BROWSEREQUEST], service_browse_input,
                             &UA_TYPES[UA_TYPES_BROWSERESPONSE], service_browse_output);
}

static const char *service_browsenext_input[] = {"continuationPoints", "releaseContinuationPoints", NULL};
static const char *service_browsenext_output[] = {"results", "responseHeader", "diagnosticInfos", NULL};
int ua_client_service_browsenext(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST], service_browsenext_input,
                             &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE], service_browsenext_output);
}

static const char *service_read_input[] = {"nodesToRead", "maxAge", "timestampsToreturn", NULL};
static const char *service_read_output[] = {"results", "responseHeader", "diagnosticInfos", NULL};
int ua_client_service_read(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_READREQUEST], service_read_input,
                             &UA_TYPES[UA_TYPES_READRESPONSE], service_read_output);
}

static const char *service_write_input[] = {"nodesToWrite", NULL};
static const char *service_write_output[] = {"results", "responseHeader", "diagnosticInfos", NULL};
int ua_client_service_write(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_WRITEREQUEST], service_write_input,
                             &UA_TYPES[UA_TYPES_WRITERESPONSE], service_write_output);
}

static const char *service_call_input[] = {"methodsToCall", NULL};
static const char *service_call_output[] = {"results", "responseHeader", "diagnosticInfos", NULL};
int ua_client_service_call(lua_State *L) {
    return ua_client_service(L, &UA_TYPES[UA_TYPES_CALLREQUEST], service_call_input,
                             &UA_TYPES[UA_TYPES_CALLRESPONSE], service_call_output);
}
