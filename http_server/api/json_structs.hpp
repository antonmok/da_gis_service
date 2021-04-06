#pragma once

#include <string>
#include <boost/json.hpp>

namespace json = boost::json;

/*******************************/
struct JSSoftwareInfo
{
    std::string serial_number;
    std::string software_version;
};
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
    unsigned long start_time;   // unix time when user allowed to operate
    unsigned long last_time;    // last time when user was logged in
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

struct JSModeAuto {
    std::vector<std::string> fines;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSModeAuto& s)
{
    jv = {
        { "fines", s.fines }
    };
}

struct JSModeAutomated {
    unsigned speedLimit;
    unsigned maxFines;
    unsigned period;
    unsigned pause;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSModeAutomated& s)
{
    jv = {
        { "speedLimit", s.speedLimit },
        { "maxFines", s.maxFines },
        { "period", s.period },
        { "pause", s.pause }
    };
}

struct JSModeMixed {
    std::vector<std::string> fines;
    unsigned speedLimit;
    unsigned maxFines;
    unsigned period;
    unsigned pause;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSModeMixed& s)
{
    jv = {
        { "fines", s.fines },
        { "maxFines", s.maxFines },
        { "pause", s.pause },
        { "period", s.period },
        { "speedLimit", s.speedLimit }
    };
}

struct JSDelayRec {
    double duration = 1;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSDelayRec& s)
{
    jv = {
        { "duration", s.duration }
    };
}

struct JSModeSettings {
    JSModeAuto auto_;
    JSModeAutomated automated;
    JSModeMixed combi;
    JSDelayRec delayRec;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSModeSettings& s)
{
    jv = {
        { "auto_", s.auto_ },
        { "automated", s.automated },
        { "combi", s.combi },
        { "delayRec", s.delayRec }
    };
}

struct JSMobile {
    bool isOn = true;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSMobile& s)
{
    jv = {
        { "isOn", s.isOn }
    };
}

struct JSGrs {
    bool sound = true;
    bool banners = true;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSGrs& s)
{
    jv = {
        { "sound", s.sound },
        { "banners", s.banners }
    };
}

struct JSNotifications {
    JSGrs grs;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSNotifications& s)
{
    jv = {
        { "grs", s.grs }
    };
}

struct JSSounds {
    std::string notify = "sound1";
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSSounds& s)
{
    jv = {
        { "notify", s.notify }
    };
}

struct JSSettingsUX {
    JSMobile mobile;
    JSNotifications notifications;
    JSSounds sounds;
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSSettingsUX& s)
{
    jv = {
        { "mobile", s.mobile },
        { "notifications", s.notifications },
        { "sounds", s.sounds },
    };
}

/*!
 * @brief: Все настройки пользователя.
 */
struct JSSettings {
  bool speed;       // включены или выключены внопки во втором тулбаре
  bool park;        //
  bool trajectory;  //
  std::vector<JSModeSettings> defaultModesSettings; // настройки режимов по умолчанию (заводские настройки, либо использованные последний раз)
  std::vector<JSModeSettings> modesSettingsPacks;   // массив сохраненных наборов настроек

  std::string lastMode; // название или id последнего использованного режима ('combi')
  JSSettingsUX settings;  // настройки уведомлений, звуков и тд
  std::vector<std::string> permissions; // объект со всей информацией, что пользователь может/не может использовать/конфигурировать (задается админом)
};

inline void tag_invoke(json::value_from_tag, json::value& jv, const JSSettings& s)
{
    jv = {
        { "speed", s.speed },
        { "park", s.park },
        { "trajectory", s.trajectory },
        { "defaultModesSettings", s.defaultModesSettings },
        { "modesSettingsPacks", s.modesSettingsPacks },
        { "lastMode", s.lastMode },
        { "settings", s.settings },
        { "permissions", s.permissions }
    };
}

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