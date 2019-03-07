# Defold uWebsockets

**Caution:** Under heavy development.

This project is using [uWebsockets 0.14](https://github.com/uNetworking/uWebSockets/tree/v0.14) and its dependencies. 

### Known problems  

- Currently MacOS and iOS only. Not support Windows and Android yet.
- Using SSL (wss) cause crash

------

### API


#### uwebsockets.connect  
uwebsockets.connect(host, name, callback, time out)

`host` "ws://" .. server_adress .. ":" .. server_port .. "/" .. room_name  
`name` connection name  
`callback` lua callback   
`time out` connection time out

#### uwebsockets.send   
uwebsockets.send(name, string)

`name` connection name  
`string` string like json 

#### uwebsockets.close
uwebsockets.send(name)  
`name` connection name  
