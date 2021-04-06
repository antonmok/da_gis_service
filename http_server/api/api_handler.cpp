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
    //route_methods_["api/login"] = LoginUser;
    route_methods_["/api/logout"] = LogoutUser;
    route_methods_["/api/secret"] = SecretTest;
    route_methods_["/api/info"] = InfoUser;
    route_methods_["/api/settings"] = Settings;
    route_methods_["/api/settings/mobile"] = SettingsMobile;
    route_methods_["/api/settings/mobile/operators"] = MobileOperator;  // POST, GET
    route_methods_["/api/settings/notifications"] = Notifications;
    route_methods_["/api/settings/sounds"] = Sounds;
    route_methods_["/api/statistics"] = Statistics;
    route_methods_["/api/map"] = Map;
    route_methods_["/api/mode-info"] = ModeInfo;
    route_methods_["/api/mode-events"] = ModeEvents;
    route_methods_["/api/event"] = Event;
    route_methods_["/api/settings/auto"] = SettingsAuto;
    route_methods_["/api/settings/automated"] = SettingsAutomated;
    route_methods_["/api/settings/combi"] = SettingsCombi;
    route_methods_["/api/settings/delay-rec"] = DelayedRecord;
    route_methods_["/api/settings/modesSettingsPacks"] = ModeSettingsPack;  // POST, DELETE
    route_methods_["/api/settings/search-grs"] = SearchGrs;
    route_methods_["/api/shortcommand"] = ShortCommand;
    route_methods_["/api/mode"] = Mode;
    route_methods_["/api/cams"] = Cams;
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
