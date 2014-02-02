// Copyright (c) 2014 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying

#include <iostream>
#include "websocketpp/config/asio_no_tls.hpp"
#include "websocketpp/server.hpp"
#include "json/json_spirit.h"
#include "json/json_spirit_stream_reader.h"

using namespace std;
using namespace json_spirit;

//define connection data
struct ConnectionData
{
    set<string> setSubList;
    Boolean isAllowed;
};


//configure websocketpp
struct CustomConfig : public websocketpp::config::asio {
    //defauts
    typedef websocketpp::config::asio core;
    
    typedef core::concurrency_type concurrency_type;
    typedef core::request_type request_type;
    typedef core::response_type response_type;
    typedef core::message_type message_type;
    typedef core::con_msg_manager_type con_msg_manager_type;
    typedef core::endpoint_msg_manager_type endpoint_msg_manager_type;
    typedef core::alog_type alog_type;
    typedef core::elog_type elog_type;
    typedef core::rng_type rng_type;
    typedef core::transport_type transport_type;
    typedef core::endpoint_base endpoint_base;
    
    //set connection data
    typedef ConnectionData connection_base;
};

typedef websocketpp::server<CustomConfig> cServer;
typedef cServer::connection_ptr connection_ptr;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class cWebsocket
{
public:
    
    
    // start the websocket server
    void run(uint16_t nPort)
    {
        server_.init_asio();
        //set event handlers
        server_.set_open_handler(bind(&cWebsocket::on_open,this,::_1));
        server_.set_close_handler(bind(&cWebsocket::on_close,this,::_1));
        server_.set_message_handler(bind(&cWebsocket::on_message,this,::_1,::_2));
        
        server_.listen(nPort);
        server_.start_accept();
        server_.run();
    }
    
    
    
    void broadcast(string event, string msg)
    {
        //for every open connection
        for(set<connection_hdl>::iterator it = setConnections_.begin(); it != setConnections_.end(); ++it) {
            connection_ptr pConnection = server_.get_con_from_hdl(*it);
            //check if user is authenticated and listens to given event
            if(pConnection->isAllowed && (pConnection->setSubList.count(event)!=0 || pConnection->setSubList.count("all") != 0))
                server_.send(*it, msg, websocketpp::frame::opcode::text);
        }
    }
    
    
    //event handlers
    void on_message(connection_hdl pHdl, cServer::message_ptr pMsg)
    {
        //parse json
        connection_ptr pConnection = server_.get_con_from_hdl(pHdl);
        mValue sMsg = pMsg->get_payload();
        mObject mapMsg = sMsg.get_obj();
        //TODO: config user and pass
        mValue sConfig = "{\"user\": \"name\", \"pass\": \"password\"}";
        mObject mapConfig = sConfig.get_obj();
        // tests if is user is allowed to alter the setSubList
        if (pConnection->isAllowed || (mapMsg.count("user") == 1 && mapMsg.count("pass") == 1 && mapMsg["user"] == mapConfig["user"] && mapMsg["pass"] == mapConfig["pass"]))
        {
            pConnection->isAllowed = true;
            pConnection->setSubList.clear();
            if (mapMsg.count("sub") == 1)
            {
                mArray mapSubs = mapMsg["sub"].get_array();
                for (unsigned int i=0; i<mapSubs.size(); i++)
                    pConnection->setSubList.insert(mapSubs[i].get_str());
            }
        }
    }
    
    

    void on_open(connection_hdl pHdl)
    {
        //inserts connection from connection set
        setConnections_.insert(pHdl);
        connection_ptr pConnection = server_.get_con_from_hdl(pHdl);
        
        pConnection->isAllowed = false;
    }
    
    

    void on_close(connection_hdl pHdl) {
        //removes connection from connection set
        setConnections_.erase(pHdl);
    }

    
    
private:
    set<connection_hdl> setConnections_;
    cServer server_;
};
