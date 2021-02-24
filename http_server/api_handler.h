#pragma once

#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include "users_storage.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

class CAPIHandler {
public:

    using route_method = std::function<bool(const std::string&, const std::string&, std::string&)>;

    static CAPIHandler& Instance();

    template<class Body, class Allocator>
    http::status HandleApiRequest(http::request<Body, http::basic_fields<Allocator>>&& req, std::string& resp_str)
    {
        std::string response;
        std::string path(req.target().data(), req.target().length());
        std::string token(req["Authorization"]);
        const auto& usersData = CUsersData::Instance();

        if (req.method() == http::verb::post && path == "/api/login") {
            // parse body json
            Credentials creds;
            if (ExtractCredentials(req.body(), creds)) {
                //
            } else {
                return http::status::unauthorized;
            }
        }

        // is user loged
        if (token == "" || token == "undefined") return http::status::unauthorized;

        // is user autorithed to call method

        // parse body as json

        // get method to call using regex

        // call

        // form http response

        resp_str.append("{\n  \"method\": \"GET\",\n  \"route\": \"");
        resp_str.append(path);
        resp_str.append("\",\n  \"message\": \"Hello\"\n}");

        return http::status::ok;
    }

    private:

    CAPIHandler();
	~CAPIHandler();

    bool ExtractCredentials(const std::string& bodyStr, Credentials& creds);
	CAPIHandler(CAPIHandler const&) = delete;
	CAPIHandler& operator= (CAPIHandler const&) = delete;

    std::map<std::string, route_method> route_methods_;
};

