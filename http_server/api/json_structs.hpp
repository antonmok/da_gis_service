#pragma once

#include <string>
#include <boost/json.hpp>

namespace json = boost::json;

/*******************************/
struct JSToken
{
    std::string token;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSToken& t)
{
    jv = {
        {"token", t.token}
    };
}

inline JSToken tag_invoke( json::value_to_tag<JSToken>, json::value const& jv )
{
    json::object const& obj = jv.as_object();

    if (obj.find("token")) {
        return JSToken{
            json::value_to<std::string>(obj.at("token")),
        };
    } else return JSToken{ "" };
}

/*******************************/
struct Credentials
{
    std::string username;
    std::string password;
};

inline Credentials tag_invoke( json::value_to_tag<Credentials>, json::value const& jv )
{
    json::object const& obj = jv.as_object();

    if (obj.find("username") && obj.find("password")) {
        return Credentials{
            json::value_to<std::string>(obj.at("username")),
            json::value_to<std::string>(obj.at("password"))
        };
    } else return Credentials{ "", "" };
}

template<typename T>
inline bool JsonToStruct(const std::string& bodyStr, T& st)
{
    json::error_code ec;
    json::value bodyJson = json::parse(bodyStr.c_str(), ec);

    if (ec) return false;

    st = json::value_to<T>(bodyJson);

    return true;
}

/*******************************/
struct JSUserInfo
{
    bool isAdmin = false;
    bool isUser = true;
    bool firstLogin = false;
    std::string name;
    std::string position;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSUserInfo& s)
{
    jv = {
        { "isAdmin", s.isAdmin },
        { "isUser", s.isUser },
        { "firstLogin", s.firstLogin },
        { "name", s.name },
        { "position", s.position }
    };
}

/*struct JS*
{
    ;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JS*& t)
{
    jv = {
        {}
    };
}*/

template<typename T>
std::string StructToJson(const T& data)
{
    json::value json_val(json::value_from(data));
    return json::serialize(json_val);
}

struct JSAuto {
    std::vector<std::string> fines;
};

struct JSMode {
    unsigned speedLimit;
    unsigned maxFines;
    unsigned period;
    unsigned pause;
};

struct JSDelayRec {
    unsigned duration;
};

struct JSModeSettings {
    JSAuto auto_;
    JSMode automated;
    JSMode combi;
    JSDelayRec delayRec;
};

struct JSMobile {
    bool isOn;
};

struct JSGrs {
    bool sound;
    bool banners;
};

struct JSNotifications {
    JSGrs grs;
};

struct JSSounds {
    std::string notify;
};

struct JSSettings {
    JSMobile mobile;
    JSNotifications notifications;
    JSSounds sounds;
};


/*!
 * @brief: Все настройки пользователя.
 */
struct JSSettingsResp {
  bool speed;       // включены или выключены внопки во втором тулбаре
  bool park;        //
  bool trajectory;  //
  std::vector<JSModeSettings> defaultModesSettings; // настройки режимов по умолчанию (заводские настройки, либо использованные последний раз)
  std::vector<JSModeSettings> modesSettingsPacks;   // массив сохраненных наборов настроек

  std::string lastMode; // название или id последнего использованного режима ('combi')
  JSSettings settings;  // настройки уведомлений, звуков и тд
  std::vector<std::string> permissions; // объект со всей информацией, что пользователь может/не может использовать/конфигурировать (задается админом)
};

/*### Мобильный интернет

route: 
method: POST,
body: {
  isOn: true/false,
  turnOff: true/false,
  turnOn: true/false
},
response: {
  success,
  isOn: true/false
},
description: команда на включение/выключение мобильного интернета. Передаю текущее состояние и команду, выключить/включить. В ответ жду сообщение о успешности и новое состояние.
Думал еще это через сокеты реализовать, но так, наверное, будет надежнее.


*/