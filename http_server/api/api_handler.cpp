#include <map>
#include <functional>
#include <iostream>

#include "url/url.hpp"
#include "jwt/json_web_tokens.h"
#include "api_handler.h"

bool GetParamKey(const std::string& path, unsigned idx, std::string& key_val);

bool LogoutUser(std::string&);
bool SecretTest(std::string&);
bool InfoUser(std::string&);

CAPIHandler& CAPIHandler::Instance()
{
	static CAPIHandler singletonInstance;
	return singletonInstance;
}

CAPIHandler::CAPIHandler()
{
    //route_methods_["api/login"] = LoginUser;
    route_methods_["/api/logout"] = LogoutUser;
    route_methods_["/api/secret"] = SecretTest;
    route_methods_["/api/info"] = InfoUser;
}

CAPIHandler::~CAPIHandler() {}

http::status CAPIHandler::LoginUser(const std::string& body, std::string& response) const
{
    // parse body json
    Credentials creds;
    if (JsonToStruct(body, creds)) {

        std::string token;
        if (CUsersData::Instance().AuthenticateUser(creds, token)) {
            // form response
            response = StructToJson(JSToken{ token });
            return http::status::ok;
        }
    } else {
        response = "Invalid json";
        return http::status::bad_request;
    }

    return http::status::unauthorized;
}

bool LogoutUser(std::string& response)
{
    CUsersData::Instance().DisproveUser(CAPIHandler::Instance().GetRequestContext().token);

    return true;
}

bool SecretTest(std::string& response)
{
    // parse body as json
    /*json::error_code ec;
    json::value js_body = json::parse(body, ec);

    if (ec) {
        resp_str.assign(ec.message());
        return false;
    }*/

    response.assign("secret method called");
    return true;
}

bool InfoUser(std::string& response)
{
    JSUserInfo js_user;
    CUsersData::UserData user_data = CUsersData::Instance().GetUserData(CAPIHandler::Instance().GetRequestContext().token);

    js_user.firstLogin = user_data.firstLogin;
    js_user.isAdmin = user_data.isAdmin;
    js_user.isUser = user_data.isUser;
    js_user.name = user_data.name;
    js_user.position = user_data.position;

    response = StructToJson(js_user);

    return true;
}

bool GetParamKey(const std::string& path, unsigned idx, std::string& key_val)
{
    // parse url, get parameters
    Url url(path);
    //auto sz = url.query().size();
    key_val.assign(url.query(idx).key());

    return true;
}

bool CAPIHandler::ExecuteRouteMethod(std::string& response) const
{
    return route_methods_.at(request_context_.route)(response);
}

void CAPIHandler::GetRequestContext(CAPIHandler::RequestContext& ctx) const
{
    ctx = request_context_;
}

const CAPIHandler::RequestContext& CAPIHandler::GetRequestContext() const
{
    return request_context_;
}
