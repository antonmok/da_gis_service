#include <map>
#include <functional>
#include <iostream>
#include <boost/json.hpp>
#include <boost/json/src.hpp>

#include "url/url.hpp"
#include "jwt/json_web_tokens.h"
#include "api_handler.h"


using route_method = std::function<bool(const std::string&, const std::string&, std::string&)>;
std::map<std::string, route_method> route_methods_;

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

void FillRouteMethods()
{
    route_methods_["api/login"] = LoginUser;
    route_methods_["api/logout"] = LogoutUser;
    route_methods_["api/secret"] = SecretTest;
    route_methods_["api/info"] = InfoUser;
}

bool GetParamKey(const std::string& path, unsigned idx, std::string& key_val)
{
    // parse url, get parameters
    Url url(path);
    //auto sz = url.query().size();
    key_val.assign(url.query(idx).key());

    return true;
}

void IsAuthorized()
{
    //auto token_boost_str = req["Authorization"];
}




