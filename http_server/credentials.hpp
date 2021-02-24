#pragma once

#include <string>
#include <boost/json.hpp>

namespace json = boost::json;

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
