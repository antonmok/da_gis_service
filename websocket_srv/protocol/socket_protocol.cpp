#include "socket_protocol.hpp"
#include "../common/logger.h"

CSocketProtocolHandler& CSocketProtocolHandler::Instance()
{
	static CSocketProtocolHandler singletonInstance;
	return singletonInstance;
}

bool CSocketProtocolHandler::HandleProtocolCommand(const std::string& request, std::string& data)
{

    return true;
}