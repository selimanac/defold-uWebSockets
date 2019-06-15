#define LIB_NAME "uWebSockets"
#define MODULE_NAME "uwebsockets"
#define DLIB_LOG_DOMAIN "uWebSockets"

#include <dmsdk/dlib/log.h>
#include <dmsdk/sdk.h>
#include <iostream>
#include <array>
#include <string>
#include <tuple>
#include "uwsclient/connection.hpp"
#include "clients/goreal/Protocol.h"

// Maximum socket connection
const int MAX_CONNECTION = 2;
// Connections
std::map<std::string, duws::Connection *> conncontainer;
// Connection class
duws::Connection *conn;
// Mutex
std::mutex m;
// Temprory ID for Queue Command
unsigned int conn_count = 0;
// Passing value from connection thread to main thread
void QueueCommand(int id, int event_id, int protocol_id, duws::MsgContainer *ws_message);

struct LuaCallbackInfo
{
    LuaCallbackInfo() : m_L(0), m_Callback(LUA_NOREF), m_Self(LUA_NOREF) {}
    lua_State *m_L;
    int m_Callback;
    int m_Self;
};

struct ConnCallback
{
    LuaCallbackInfo m_Callback;

    ConnCallback()
    {
        memset(this, 0, sizeof(*this));
        m_Callback.m_Callback = LUA_NOREF;
        m_Callback.m_Self = LUA_NOREF;
    }

    void Delete() {}
};

struct MessageCommand
{
    int m_Id;
    int m_Event_Id;
    int m_Protocol_Id;
    duws::MsgContainer *m_ws_message;
};

struct ConnectionState
{
    ConnCallback m_Connections[MAX_CONNECTION];
    dmArray<MessageCommand> m_CmdQueue;
};

ConnectionState *g_Conn = 0;

void QueueCommand(int id, int event_id, int protocol_id, duws::MsgContainer *ws_message)
{
    MessageCommand cmd;
    cmd.m_Id = id;
    cmd.m_Event_Id = event_id;
    cmd.m_Protocol_Id = protocol_id;
    cmd.m_ws_message = ws_message;

    std::lock_guard<std::mutex> g(m);
    if (g_Conn->m_CmdQueue.Full())
    {
        g_Conn->m_CmdQueue.OffsetCapacity(8);
    }
    g_Conn->m_CmdQueue.Push(cmd);
}

static void RegisterCallback(lua_State *L, int index, LuaCallbackInfo *cbk)
{
    if (cbk->m_Callback != LUA_NOREF)
    {
        dmLogInfo("1- LUA_NOREF");
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Callback);
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Self);
    }

    cbk->m_L = dmScript::GetMainThread(L);
    luaL_checktype(L, index, LUA_TFUNCTION);

    lua_pushvalue(L, index);
    cbk->m_Callback = dmScript::Ref(L, LUA_REGISTRYINDEX);

    dmScript::GetInstance(L);
    cbk->m_Self = dmScript::Ref(L, LUA_REGISTRYINDEX);
}

static void InvokeCallback(LuaCallbackInfo *cbk, MessageCommand *cmd)
{
   
   m.lock();

    if (cbk->m_Callback == LUA_NOREF)
    {
        dmLogInfo("2-LUA_NOREF");
        return;
    }
   

    lua_State *L = cbk->m_L;
    DM_LUA_STACK_CHECK(L, 0);

    lua_rawgeti(L, LUA_REGISTRYINDEX, cbk->m_Callback);
    lua_rawgeti(L, LUA_REGISTRYINDEX, cbk->m_Self);
    lua_pushvalue(L, -1);

    dmScript::SetInstance(L);

   // ConnCallback *ad = &g_Conn->m_Connections[cmd->m_Id];

    /*
    Main Table
    -array - element -*/
    lua_createtable(L, 0, 0); // Main Table

    // Event ID is for client status. Protocol id used for server like Colyseus
    lua_pushnumber(L, cmd->m_Event_Id);
    lua_setfield(L, -2, "event_id");

    lua_pushnumber(L, cmd->m_Protocol_Id);
    lua_setfield(L, -2, "protocol_id");

   

    //DEBUG
    std::string numStr = std::to_string(cmd->m_Event_Id);
  

    if (cmd->m_Event_Id == Events::ON_MESSAGE)
    {
       
        if (conncontainer["general"]->connected() == true)
        {
          
            
            lua_pushstring(L, cmd->m_ws_message->jsonstring);
            lua_setfield(L, -2, "json");
        }
    }
  
    int number_of_arguments = 2; // instance + 1
    int ret = lua_pcall(L, number_of_arguments, 0, 0);
    if (ret != 0)
    {
        dmLogError("Error running callback: %s", lua_tostring(L, -1));
        lua_pop(L, 1);
    }
  m.unlock();
}

static void FlushCommandQueue()
{

    for (uint32_t i = 0; i != g_Conn->m_CmdQueue.Size(); ++i)
    {
    
        MessageCommand *cmd = &g_Conn->m_CmdQueue[i];
        ConnCallback &ad = g_Conn->m_Connections[cmd->m_Id];

        InvokeCallback(&ad.m_Callback, cmd);

        g_Conn->m_CmdQueue.EraseSwap(i--);
    }
}

static void UnregisterCallback(LuaCallbackInfo *cbk)
{
    if (cbk->m_Callback != LUA_NOREF)
    {
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Callback);
        dmScript::Unref(cbk->m_L, LUA_REGISTRYINDEX, cbk->m_Self);
        cbk->m_Callback = LUA_NOREF;
    }
}

static void DeleteCommandCallback(int type)
{
    ConnCallback *ad = &g_Conn->m_Connections[type];
    UnregisterCallback(&ad->m_Callback);
    ad->Delete();
}

/************************/
/*      CONNECTIONS     */
/************************/

//Connect to WS
void uwsconnect(std::string name)
{
    conncontainer[name]->connect(name);
}

//Join to room
void uwsjoin(std::string name, std::string room)
{
    if (conncontainer.count(name))
    {
        conncontainer[name]->join(room);
    }
    else
    {
        printf("WARNING!!: %s connection is not available\n", name.c_str());
    }
}

void uwssend(std::string name, lua_State *L)
{
    if (conncontainer[name]->connected() == false)
    {
        return;
    }

    if (conncontainer.count(name))
    {
        conncontainer[name]->send(L);
    }
    else
    {
        printf("WARNING!!: %s connection is not available\n", name.c_str());
    }
}

//Close connection
void uwsclose(std::string name)
{
    if (conncontainer.count(name))
    {
        delete conncontainer[name];
        conncontainer[name] = NULL;
        conncontainer.erase(name);
    }
    else
    {
        printf("WARNING!!: %s connection is not available\n", name.c_str());
    }
}

/*************************/
/*      LUA METHODS      */
/*************************/

static int close(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);
    std::string conn_name = luaL_checkstring(L, 1);
    if (conncontainer.count(conn_name))
    {
        conn_count = conncontainer[conn_name]->id;
        DeleteCommandCallback(conncontainer[conn_name]->id);
        uwsclose(conn_name);
    }
    else
    {
        printf("WARNING!!: %s connection is not available\n", conn_name.c_str());
    }
    return 0;
}

static int send(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    std::string conn_name = luaL_checkstring(L, 1);

    if (conncontainer.count(conn_name))
    {
        uwssend(conn_name, L);
    }
    else
    {
        printf("WARNING!!: %s connection is not available\n", conn_name.c_str());
    }

    return 0;
}

static int join(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);
    std::string conn_name = luaL_checkstring(L, 1);
    std::string room_name = luaL_checkstring(L, 2);
    uwsjoin(conn_name, room_name);
    return 0;
}

static int connect(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);

    std::string conn_name = luaL_checkstring(L, 2);
    if (conncontainer.count(conn_name))
    {
        printf("WARNING!!: %s connection is already open\n", conn_name.c_str());
    }
    else
    {
        //Just for 2 threads
        if (conn_count == MAX_CONNECTION)
        {
            conn_count = 0;
        }
        // New Connection
        conn = new duws::Connection();
        conncontainer[conn_name] = conn;
        conncontainer[conn_name]->ws_url = luaL_checkstring(L, 1);
        conncontainer[conn_name]->ws_timeout = luaL_checkint(L, 4);
        conncontainer[conn_name]->id = (conn_count);

        ConnCallback *ad = &g_Conn->m_Connections[conn_count];
        RegisterCallback(L, 3, &ad->m_Callback);

        conn_count += 1;

        uwsconnect(conn_name);
    }

    return 0;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
    {
        {"connect", connect},
        {"close", close},
        {"join", join},
        {"send", send},
        {0, 0}};

static void LuaInit(lua_State *L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name)                           \
    lua_pushnumber(L, (lua_Number)Protocols::name); \
    lua_setfield(L, -2, #name);

    // User-related (0~10)
    SETCONSTANT(USER_ID);

    // Room-related (10~20)
    SETCONSTANT(JOIN_ROOM);
    SETCONSTANT(JOIN_ERROR);
    SETCONSTANT(LEAVE_ROOM);
    SETCONSTANT(ROOM_DATA);
    SETCONSTANT(ROOM_STATE);
    SETCONSTANT(ROOM_STATE_PATCH);

    /// Match-making related (20~29)
    SETCONSTANT(ROOM_LIST);

    // Generic messages (50~60)
    SETCONSTANT(BAD_REQUEST);
#undef SETCONSTANT

#define SETCONSTANT(name)                        \
    lua_pushnumber(L, (lua_Number)Events::name); \
    lua_setfield(L, -2, #name);

    // EVENTS (60-70)
    SETCONSTANT(ON_ERROR);
    SETCONSTANT(ON_CONNECTION);
    SETCONSTANT(ON_MESSAGE);
    SETCONSTANT(ON_DISCONNECT);
#undef SETCONSTANT
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result UpdateDuwsExtension(dmExtension::Params *params)
{
    if (g_Conn)
    {
        FlushCommandQueue();
    }
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppInitializeDuwsExtension(dmExtension::AppParams *params)
{
    g_Conn = new ConnectionState;
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeDuwsExtension(dmExtension::Params *params)
{
    // Init Lua
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeDuwsExtension(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeDuwsExtension(dmExtension::Params *params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(uWebSockets, LIB_NAME, AppInitializeDuwsExtension, AppFinalizeDuwsExtension, InitializeDuwsExtension, UpdateDuwsExtension, 0, FinalizeDuwsExtension)
