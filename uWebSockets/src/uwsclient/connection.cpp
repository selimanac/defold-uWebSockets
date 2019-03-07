#include "connection.hpp"

extern void QueueCommand(int id, int event_id, int protocol_id, duws::MsgContainer *ws_message);

namespace duws
{
//init
Connection::Connection()
{
    isConnected = false;
}

//Destructor
Connection::~Connection()
{
   // If connected close the connection
    if (isConnected)
    {
        client->close();
    }
    //Join the thread
    if (uws_thread->joinable())
    {
        uws_thread->join();
    }
    //Delete thread
    delete uws_thread;
    uws_thread = NULL;
}

void Connection::send(lua_State *L)
{
    DM_LUA_STACK_CHECK(L, 0);
    const char *msg = luaL_checkstring(L, 2);
    client->send(msg);
}

//Join Room
void Connection::join(std::string room)
{
    //Implement your method here
}

//Force close
void Connection::close()
{
    this->~Connection();
}

//Connect to WS
void Connection::connect(std::string conn_name)
{
    //Assign a name to thread
    thread_name = conn_name;
    // Start a thread
    uws_thread = new std::thread(&Connection::uwsthread, this);
}

void Connection::uwsthread()
{
    //Assign a name to thread
    std::string s = "duws-" + thread_name;
    pthread_setname_np(s.c_str());

    //New Hub
    Hub h;

    //OnError
    h.onError([this](void *user) {
        //Set connection false
        isConnected = false;
        //Invoke callback
        QueueCommand(Connection::id, Events::ON_ERROR, 0, nullptr);
    });

    //OnConnection
    h.onConnection([this](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) { 
        //Assign to cliect
        client = ws;
        //Set connection true
        isConnected = true;
        //Invoke callback
        QueueCommand(Connection::id, Events::ON_CONNECTION, 0, nullptr);
    });

    //OnMessage
    h.onMessage([this](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
      
        if (message != nullptr)
        {
            message[length] = '\0';
        } else {
            message = "There is a problem. Are you sending to fast???";
        }

        msgcontainer->roomname = "RoomName";
        msgcontainer->jsonstring = message;

        QueueCommand(Connection::id, Events::ON_MESSAGE, Protocols::ROOM_STATE_PATCH, msgcontainer);
    });

    //OnDisconnection
    h.onDisconnection([this](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
        isConnected = false;
        QueueCommand(Connection::id, Events::ON_DISCONNECT, 0, nullptr);
    });

    //Connect to WS with time out
    h.connect(ws_url, nullptr, {}, ws_timeout);
    h.run();
}

} // namespace duws