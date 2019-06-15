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
    //TODO: empty QueueCommands

    // If connected close the connection
    if (isConnected)
    {
        client->terminate();
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
    if (isConnected)
    {
        DM_LUA_STACK_CHECK(L, 0);
        const char *msg = luaL_checkstring(L, 2);
        client->send(msg, strlen(msg), opcode);
    }
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

bool Connection::connected()
{
    return isConnected;
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

    std::mutex holdUp;

    //OnError
    h.onError([this](void *user) {
        dmLogInfo("H -> onError");
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
    h.onMessage([this, &holdUp](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
        holdUp.lock();
        dmLogInfo("H -> onMessage");
        dmLogInfo(message);
        if (message != nullptr || message != NULL)
        {
            message[length] = '\0'; //"\uFFFD"

            msgcontainer->roomname = "RoomName";
            msgcontainer->jsonstring = message;

            QueueCommand(Connection::id, Events::ON_MESSAGE, Protocols::ROOM_STATE_PATCH, msgcontainer);
        }
        else
        {
            dmLogInfo("5- Null geldi");
        }

        holdUp.unlock();
    });

    //OnDisconnection
    h.onDisconnection([this, &holdUp](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
        holdUp.lock();
        dmLogInfo("H -> onDisconnection");

        std::string numStr = std::to_string(code);
        dmLogInfo(numStr.c_str());

        isConnected = false;
        QueueCommand(Connection::id, Events::ON_DISCONNECT, 0, nullptr);
        holdUp.unlock();
    });

    //Connect to WS with time out
    h.connect(ws_url, nullptr, {}, ws_timeout);
    h.run();
    h.poll();
}

} // namespace duws