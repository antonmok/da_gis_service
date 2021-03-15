#pragma once

#include <mutex>
#include <string>
#include <boost/json.hpp>
#include <string_view>

const unsigned controlPortNumber = 8083;
const unsigned streamingPortNumber = 8084;

using namespace std::literals;

struct CCommandNames
{
    static constexpr auto startStreaming = "StartStreaming"sv;
    static constexpr auto stopStreaming = "StopStreaming"sv;
};

// TODO: RnD
enum class SocketControlCmd {
    notDefined,
    startStreaming,
    stopStreaming,
    getFrame,
    /*setBrightness,
    setMode,
    setSettings,
    getSettings,*/

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

    SocketControlCmd ParseRequest(const std::string) const;
    
    mutable std::mutex mutex_;

};

