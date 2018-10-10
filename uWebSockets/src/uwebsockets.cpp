// myextension.cpp
// Extension lib defines
#define LIB_NAME "uWebSockets"
#define MODULE_NAME "uwebsockets"

// include the Defold SDK
#include <dmsdk/sdk.h>

#include <uWS/uWS.h>
#include <iostream>
#include <chrono>
#include <cmath>
#include <thread>
#include <fstream>
#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <map>
#include <atomic>

#define THREAD_IMPLEMENTATION
#include "thread.h"

using namespace uWS;

int thread_proc(void *user_data)
{
    Hub h;
    std::string response = "I am a stupid web service in Defold";
    
    h.onMessage([](WebSocket<SERVER> *ws, char *message, size_t length, OpCode opCode) {
        ws->send(message, length, opCode);
    });
    
    h.onHttpRequest([&](HttpResponse *res, HttpRequest req, char *data, size_t length,
                        size_t remainingBytes) {
        res->end(response.data(), response.length());
    });
    
    if (h.listen(3000)) {
        h.run();
    }
    printf("Going out the thread\n");
    return 0;
}

static int StartServer(lua_State *L)
{
    int top = lua_gettop(L);

    std::vector<std::thread *> threads(std::thread::hardware_concurrency());
    std::transform(threads.begin(), threads.end(), threads.begin(), [](std::thread *t) {
        return new std::thread([]() {
            Hub h;
            std::string response = "I am a stupid web service embeded in Defold!";
            
            h.onMessage([](WebSocket<SERVER> *ws, char *message, size_t length, OpCode opCode) {
                ws->send(message, length, opCode);
            });
            
            h.onHttpRequest([&](HttpResponse *res, HttpRequest req, char *data, size_t length,
                                size_t remainingBytes) {
                res->end(response.data(), response.length());
            });
            
            if (h.listen(3000)) {
                h.run();
            }
        });
    });
































/* 
    thread_atomic_int_t exit_flag;
    thread_atomic_int_store(&exit_flag, 0);

    thread_ptr_t thread = thread_create(thread_proc, &exit_flag, "Example thread", THREAD_STACK_SIZE_DEFAULT); */

   // int retval = thread_join(thread);

    printf("Out of the thread\n");

   // thread_destroy(thread);
    printf("thread destroyed\n");
    /* 
    std::vector<std::thread *> threads(std::thread::hardware_concurrency());
    std::transform(threads.begin(), threads.end(), threads.begin(), [](std::thread *t) {
        return new std::thread([]() {
            Hub h;
            std::string response = "Hello Monkey Me!";
            
            h.onMessage([](WebSocket<SERVER> *ws, char *message, size_t length, OpCode opCode) {
                ws->send(message, length, opCode);
            });
            
            h.onHttpRequest([&](HttpResponse *res, HttpRequest req, char *data, size_t length,
                                size_t remainingBytes) {
                res->end(response.data(), response.length());
            });
            
            if (h.listen(3000)) {
                h.run();
            }
        });
    });
    

    std::for_each(threads.begin(), threads.end(), [](std::thread *t) {
        t->join();
    }); */

    /* 
    uWS::Hub h;

    h.onError([](void *user) {
        switch ((long) user) {
        case 1:
            std::cout << "Client emitted error on invalid URI" << std::endl;
            break;
        case 2:
            std::cout << "Client emitted error on resolve failure" << std::endl;
            break;
        case 3:
            std::cout << "Client emitted error on connection timeout (non-SSL)" << std::endl;
            break;
        case 5:
            std::cout << "Client emitted error on connection timeout (SSL)" << std::endl;
            break;
        case 6:
            std::cout << "Client emitted error on HTTP response without upgrade (non-SSL)" << std::endl;
            break;
        case 7:
            std::cout << "Client emitted error on HTTP response without upgrade (SSL)" << std::endl;
            break;
        case 10:
            std::cout << "Client emitted error on poll error" << std::endl;
            break;
        case 11:
            static int protocolErrorCount = 0;
            protocolErrorCount++;
            std::cout << "Client emitted error on invalid protocol" << std::endl;
            if (protocolErrorCount > 1) {
                std::cout << "FAILURE:  " << protocolErrorCount << " errors emitted for one connection!" << std::endl;
                exit(-1);
            }
            break;
        default:
            std::cout << "FAILURE: " << user << " should not emit error!" << std::endl;
            exit(-1);
        }
    });

    h.onConnection([](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
        switch ((long) ws->getUserData()) {
        case 8:
            std::cout << "Client established a remote connection over non-SSL" << std::endl;
            ws->close(1000);
            break;
        case 9:
            std::cout << "Client established a remote connection over SSL" << std::endl;
            ws->close(1000);
            break;
        default:
            std::cout << "FAILURE: " << ws->getUserData() << " should not connect!" << std::endl;
            exit(-1);
        }
    });

    h.onDisconnection([](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
        std::cout << "Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << std::string(message, length) << ">" << std::endl;
    });

    h.connect("invalid URI", (void *) 1);
    h.connect("invalid://validButUnknown.yolo", (void *) 11);
    h.connect("ws://validButUnknown.yolo", (void *) 2);
    h.connect("ws://echo.websocket.org", (void *) 3, {}, 10);
    h.connect("ws://echo.websocket.org", (void *) 8);
    h.connect("wss://echo.websocket.org", (void *) 5, {}, 10);
    h.connect("wss://echo.websocket.org", (void *) 9);
    h.connect("ws://google.com", (void *) 6);
    h.connect("wss://google.com", (void *) 7);
    h.connect("ws://127.0.0.1:6000", (void *) 10, {}, 60000);

    h.run();
    std::cout << "Falling through testConnections" << std::endl;
     */

    /* Hub h;
    std::string response = "Hello from defold!";
    
    h.onMessage([](WebSocket<SERVER> *ws, char *message, size_t length, OpCode opCode) {
        ws->send(message, length, opCode);
    });
    
    h.onHttpRequest([&](HttpResponse *res, HttpRequest req, char *data, size_t length,
                        size_t remainingBytes) {
        res->end(response.data(), response.length());
    });
    
    if (h.listen(3000)) {
        h.run();
    } */

    return 0;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] =
    {
        {"start_server", StartServer},
        {0, 0}};

static void LuaInit(lua_State *L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result AppInitializeMyExtension(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result InitializeMyExtension(dmExtension::Params *params)
{
    // Init Lua
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeMyExtension(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result FinalizeMyExtension(dmExtension::Params *params)
{
    return dmExtension::RESULT_OK;
}

// Defold SDK uses a macro for setting up extension entry points:
//
// DM_DECLARE_EXTENSION(symbol, name, app_init, app_final, init, update, on_event, final)

DM_DECLARE_EXTENSION(uWebSockets, LIB_NAME, AppInitializeMyExtension, AppFinalizeMyExtension, InitializeMyExtension, 0, 0, FinalizeMyExtension)
