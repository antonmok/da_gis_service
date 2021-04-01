#include "socket_protocol.hpp"
#include "shared_image/shared_image.hpp"
#include "../common/logger.h"

CSocketProtocolHandler& CSocketProtocolHandler::Instance()
{
	static CSocketProtocolHandler singletonInstance;
	return singletonInstance;
}

bool utf8_check_is_valid(const std::string& string)
{
    int c,i,ix,n,j;
    for (i=0, ix=string.length(); i < ix; i++)
    {
        c = (unsigned char) string[i];
        //if (c==0x09 || c==0x0a || c==0x0d || (0x20 <= c && c <= 0x7e) ) n = 0; // is_printable_ascii
        if (0x00 <= c && c <= 0x7f) n=0; // 0bbbbbbb
        else if ((c & 0xE0) == 0xC0) n=1; // 110bbbbb
        else if ( c==0xed && i<(ix-1) && ((unsigned char)string[i+1] & 0xa0)==0xa0) return false; //U+d800 to U+dfff
        else if ((c & 0xF0) == 0xE0) n=2; // 1110bbbb
        else if ((c & 0xF8) == 0xF0) n=3; // 11110bbb
        //else if (($c & 0xFC) == 0xF8) n=4; // 111110bb //byte 5, unnecessary in 4 byte UTF-8
        //else if (($c & 0xFE) == 0xFC) n=5; // 1111110b //byte 6, unnecessary in 4 byte UTF-8
        else return false;
        for (j=0; j<n && i<ix; j++) { // n bytes matching 10bbbbbb follow ?
            if ((++i == ix) || (( (unsigned char)string[i] & 0xC0) != 0x80))
                return false;
        }
    }
    return true;
}

bool CSocketProtocolHandler::HandleProtocolCommand(const std::string& request, std::string& data)
{
    SocketControlCmd cmd = ParseRequest(request);

    if (cmd == SocketControlCmd::startStreaming) {
        // start sending
    } else if (cmd == SocketControlCmd::getFrame) {
        CSharedImage::Instance().GetBase64Frame(data);
        if (utf8_check_is_valid(data)) {
            return true;
        } else {
            LOG(ERROR) << "not utf8 chars in base64";
        }
    }
    return false;
}

SocketControlCmd CSocketProtocolHandler::ParseRequest(const std::string request) const
{
    if (request == CCommandNames::startStreaming) {
        return SocketControlCmd::startStreaming;
    } else if (request == CCommandNames::stopStreaming) {
        return SocketControlCmd::stopStreaming;
    } else if (request == CCommandNames::getFrame) {
        return SocketControlCmd::getFrame;
    } else {
        return SocketControlCmd::notDefined;
    }
}
