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

http::status handle_api_request(const std::string& path, std::string& resp_str)
{
    // parse url, get parameters
    Url url(path);
    auto sz = url.query().size();
    // is user loged

    // is user autorithed to call method

    // get method to call using regex

    // call

    // form http response

    resp_str.append("{\n  \"method\": \"GET\",\n  \"route\": \"");
    resp_str.append(path.data(), path.size());
    resp_str.append("\",\n  \"message\": \"Hello\"\n}");

    return http::status::ok;
}

http::status handle_api_post_request(const std::string& path, const std::string& body, std::string& resp_str)
{
    // parse body as json

    // is user loged

    // is user autorithed to call method

    // get method to call using regex

    // call

    // form http response

    resp_str.append("{\n  \"method\": \"POST\",\n  \"route\": \"");
    resp_str.append(path.data(), path.size());
    resp_str.append("\",\n  \"message\": \"Hello\"\n}");

    return http::status::ok;
}

