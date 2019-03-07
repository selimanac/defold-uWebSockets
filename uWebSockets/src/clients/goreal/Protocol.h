/* Used for Colyseus*/ 
#ifndef Protocol_h
#define Protocol_h

enum Protocols
{
    // User-related (0~10)
    USER_ID = 1,

    // Room-related (10~20)
    JOIN_ROOM = 10,
    JOIN_ERROR = 11,
    LEAVE_ROOM = 12,
    ROOM_DATA = 13,
    ROOM_STATE = 14,
    ROOM_STATE_PATCH = 15,

    // Match-making related (20~29)
    ROOM_LIST = 20,

    // Generic messages (50~60)
    BAD_REQUEST = 50,
};

enum Events
{
    // EVENTS (60-70)
    ON_ERROR = 60,
    ON_CONNECTION = 61,
    ON_MESSAGE = 62,
    ON_DISCONNECT = 63

};

#endif /* Protocol_h */
