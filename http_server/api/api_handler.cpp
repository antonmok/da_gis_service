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
bool Settings(std::string&);
bool SettingsMobile(std::string&);
bool MobileOperator(std::string&);
bool Notifications(std::string&);
bool Sounds(std::string&);
bool Statistics(std::string&);
bool Map(std::string&);
bool ModeInfo(std::string&);
bool Mode(std::string&);
bool ModeEvents(std::string&);
bool Event(std::string&);
bool SettingsAuto(std::string&);
bool SettingsAutomated(std::string&);
bool SettingsCombi(std::string&);
bool DelayedRecord(std::string&);
bool ModeSettingsPack(std::string&);
bool SearchGrs(std::string&);
bool ShortCommand(std::string&);
bool Cams(std::string&);

CAPIHandler& CAPIHandler::Instance()
{
    static CAPIHandler singletonInstance;
    return singletonInstance;
}

CAPIHandler::CAPIHandler()
{
    //route_methods_["api/login"] = { LoginUser, { http::verb::post } };
    route_methods_["/api/logout"] = { LogoutUser, { http::verb::post } };
    route_methods_["/api/secret"] = { SecretTest, { http::verb::get } };
    route_methods_["/api/info"] = { InfoUser, { http::verb::get } };
    route_methods_["/api/settings"] = { Settings, { http::verb::get } };
    route_methods_["/api/settings/mobile"] = { SettingsMobile, { http::verb::post } };
    route_methods_["/api/settings/mobile/operators"] = { MobileOperator, { http::verb::post, http::verb::get } };
    route_methods_["/api/settings/notifications"] = { Notifications, { http::verb::post } };
    route_methods_["/api/settings/sounds"] = { Sounds, { http::verb::post } };
    route_methods_["/api/statistics"] = { Statistics, { http::verb::get } };
    route_methods_["/api/map"] = { Map, { http::verb::get } };
    route_methods_["/api/mode-info"] = { ModeInfo, { http::verb::get } };
    route_methods_["/api/mode-events"] = { ModeEvents, { http::verb::get } };
    route_methods_["/api/event"] = { Event, { http::verb::get } };
    route_methods_["/api/settings/auto"] = { SettingsAuto, { http::verb::post } };
    route_methods_["/api/settings/automated"] = { SettingsAutomated, { http::verb::post } };
    route_methods_["/api/settings/combi"] = { SettingsCombi, { http::verb::post } };
    route_methods_["/api/settings/delay-rec"] = { DelayedRecord, { http::verb::post } };
    route_methods_["/api/settings/modesSettingsPacks"] = { ModeSettingsPack, { http::verb::post, http::verb::delete_ } };
    route_methods_["/api/settings/search-grs"] = { SearchGrs, { http::verb::get } };
    route_methods_["/api/shortcommand"] = { ShortCommand, { http::verb::post } };
    route_methods_["/api/mode"] = { Mode, { http::verb::post } };
    route_methods_["/api/cams"] = { Cams, { http::verb::post } };
}

CAPIHandler::~CAPIHandler() {}

/********* API METHODS ********/

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

bool Settings(std::string& response)
{
    CUsersData::UserData user_data = CUsersData::Instance().GetUserData(CAPIHandler::Instance().GetRequestContext().token);

    response = StructToJson(user_data.settings);

    return true;
}

bool SettingsMobile(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool MobileOperator(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool Notifications(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool Sounds(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool Statistics(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool Map(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool ModeInfo(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool Mode(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool ModeEvents(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool Event(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool SettingsAuto(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool SettingsAutomated(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool SettingsCombi(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool DelayedRecord(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool ModeSettingsPack(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool SearchGrs(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool ShortCommand(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

bool Cams(std::string& response)
{
    response = "{\"data:\" \"empty\"}";

    return true;
}

/*************************************/

bool GetParamKey(const std::string& path, unsigned idx, std::string& key_val)
{
    // parse url, get parameters
    Url url(path);
    //auto sz = url.query().size();
    key_val.assign(url.query(idx).key());

    return true;
}

bool CAPIHandler::MethodAllowed(http::verb method, std::string& route) const
{
    for (const auto& allowed_method : route_methods_.at(route).allowed_methods) {
        if (method == allowed_method) return true;
    }

    return false;
}

bool CAPIHandler::ExecuteRouteMethod(std::string& response) const
{
    return route_methods_.at(request_context_.route).func(response);
}

void CAPIHandler::GetRequestContext(CAPIHandler::RequestContext& ctx) const
{
    ctx = request_context_;
}

const CAPIHandler::RequestContext& CAPIHandler::GetRequestContext() const
{
    return request_context_;
}
