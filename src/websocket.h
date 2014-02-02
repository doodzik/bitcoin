// Copyright (c) 2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying

#ifndef BITCOIN_WEBSOCKET_H
#define BITCOIN_WEBSOCKET_H

struct ConnectionData{};
struct CustomConfig{};

class cWebsocket
{
public:
    void run(u_int16_t port){}
    void broadcast(){}
    void on_message(){}
    void on_open(){}
    void on_close(){}
};

#endif
