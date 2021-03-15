#include "socket_protocol.hpp"
#include "../common/logger.h"

CSocketProtocolHandler& CSocketProtocolHandler::Instance()
{
	static CSocketProtocolHandler singletonInstance;
	return singletonInstance;
}

bool CSocketProtocolHandler::HandleProtocolCommand(const std::string& request, std::string& data)
{
    SocketControlCmd cmd = ParseRequest(request);

    if (cmd == SocketControlCmd::startStreaming) {
        // start sending
    } else if (cmd == SocketControlCmd::getFrame) {
        // TODO: implement first
    }
    return false;
}

SocketControlCmd CSocketProtocolHandler::ParseRequest(const std::string request) const
{
    if (request == CCommandNames::startStreaming) {
        return SocketControlCmd::startStreaming;
    } else if (request == CCommandNames::stopStreaming) {
        return SocketControlCmd::stopStreaming;
    } else {
        return SocketControlCmd::notDefined;
    }
}
