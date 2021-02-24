#include <map>
#include <functional>
#include <iostream>

#include "url/url.hpp"
#include "jwt/json_web_tokens.h"
#include "api_handler.h"

bool GetParamKey(const std::string& path, unsigned idx, std::string& key_val);

bool LoginUser(const std::string& path, const std::string& body, std::string& resp_str);
bool LogoutUser(const std::string& path, const std::string& body, std::string& resp_str);
bool SecretTest(const std::string& path, const std::string& body, std::string& resp_str);
bool InfoUser(const std::string& path, const std::string& body, std::string& resp_str);

CAPIHandler& CAPIHandler::Instance()
{
	static CAPIHandler singletonInstance;
	return singletonInstance;
}

CAPIHandler::CAPIHandler()
{
    route_methods_["api/login"] = LoginUser;
    route_methods_["api/logout"] = LogoutUser;
    route_methods_["api/secret"] = SecretTest;
    route_methods_["api/info"] = InfoUser;
}

CAPIHandler::~CAPIHandler() {}

bool CAPIHandler::ExtractCredentials(const std::string& bodyStr, Credentials& creds)
{
    json::error_code ec;
    json::value bodyJson = json::parse(bodyStr.c_str(), ec);

    if (ec) return false;

    creds = json::value_to<Credentials>(bodyJson);

    return true;
}

bool LoginUser(const std::string& path, const std::string& body, std::string& resp_str)
{
    return true;
}

bool LogoutUser(const std::string& path, const std::string& body, std::string& resp_str)
{
    return true;
}

bool SecretTest(const std::string& path, const std::string& body, std::string& resp_str)
{
    return true;
}

bool InfoUser(const std::string& path, const std::string& body, std::string& resp_str)
{
    return true;
}

bool ParseBodyJson(const std::string& bodyStr, json::value& bodyJson);

bool GetParamKey(const std::string& path, unsigned idx, std::string& key_val)
{
    // parse url, get parameters
    Url url(path);
    //auto sz = url.query().size();
    key_val.assign(url.query(idx).key());

    return true;
}




