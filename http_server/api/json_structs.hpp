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
inline bool JsonToStruct(const std::string& bodyStr, T& creds)
{
    json::error_code ec;
    json::value bodyJson = json::parse(bodyStr.c_str(), ec);

    if (ec) return false;

    creds = json::value_to<T>(bodyJson);

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