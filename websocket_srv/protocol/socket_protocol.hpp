#pragma once

#include <mutex>
#include <string>
#include <boost/json.hpp>

const unsigned controlPortNumber = 8083;
const unsigned streamingPortNumber = 8084;

// TODO: RnD
enum class SocketControlCmd {
    StartStreaming,
    StopStreaming,
    SetBrightness,
    SetMode,
    SetSettings,
    GetSettings,

};

// TODO: RnD
struct SocketData {
    bool statistics;
    bool reports;
    bool events;
    bool map;
    bool notifications;

};


class CSocketProtocolHandler {

public:

    static CSocketProtocolHandler& Instance();

    bool HandleProtocolCommand(const std::string& request, std::string& data);


private:

    CSocketProtocolHandler() {};
	~CSocketProtocolHandler() {};

	CSocketProtocolHandler(CSocketProtocolHandler const&) = delete;
	CSocketProtocolHandler& operator= (CSocketProtocolHandler const&) = delete;

    
    mutable std::mutex mutex_;

};

