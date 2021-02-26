#include <string>
#include <iostream>
#include <boost/json.hpp>
#include <boost/json/src.hpp> // for header only version

namespace json = boost::json;

struct Credentials
{
    std::string username;
    std::string password;
};

struct Test
{
    std::string token;
};

void tag_invoke(json::value_from_tag, json::value& jv, const Test& t)
{
    jv = {
        {"token", t.token}
    };

    std::cout << jv.as_object()["token"] << std::endl;
}

template<typename T>
std::string StructToJson(const T& data)
{
    json::value json_val(json::value_from(data));
    return json::serialize(json_val);
}

int main()
{
    Test t;
    t.token = "werwerwerwerwer";

    /*json::value t_json(json::value_from(t));
    
    std::string jsstr = json::serialize(t_json);*/

    std::cout << StructToJson(t) << std::endl;

    return 0;
}