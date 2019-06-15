
#ifndef connection_hpp
#define connection_hpp

#include <uWS/uWS.h>
#include <iostream>
#include <array>
#include <string>
#include <thread>
#include <functional>
#include <pthread.h>
#include <sstream>
//#include "../clients/colyseus/MsgParser.hpp"
#include "../clients/goreal/Protocol.h"
#include <dmsdk/sdk.h>

using namespace uWS;

namespace duws
{

//Generic State Message
struct MsgContainer
{
  std::string roomname;
  std::string sessionid;
  int protocolID;
  char *jsonstring;
};

class Connection
{
public:
  //Default constructor
  Connection();
  //Destructor prototype
  ~Connection();
  //Connect to WS
  void connect(std::string conn_name);
  //Force close if connected
  void close();
  //Send message
  void send(lua_State *L);
  //Join Room
  void join(std::string room);
  //WS Address
  std::string ws_url;
  //Connection Timeout
  unsigned int ws_timeout;
  //Connection ID
  size_t id;
  bool connected();

private:
  //Parser Class
  MsgContainer *msgcontainer = new MsgContainer;
  //New thread
  std::thread *uws_thread;
  //Thread method
  void uwsthread();
  //Thread name
  std::string thread_name;
  //Check if connected
  std::atomic<bool> isConnected;
  //uws client
  uWS::WebSocket<uWS::CLIENT> *client;
  // OpCode Binary
  uWS::OpCode opcode = BINARY;
  //Room name
  std::string roomname;
  // User session ID
  std::string sessionid;
};
} // namespace duws

#endif /* connection_hpp */
