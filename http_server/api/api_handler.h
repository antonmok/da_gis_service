#pragma once

#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include "users/users_storage.h"
#include "json_structs.hpp"

namespace beast = boost::beast;
namespace http = beast::http;

class CAPIHandler {
public:

    static CAPIHandler& Instance();

    struct RequestContext
    {
        std::string route;
        std::string token;
        std::string body;
    };

    void GetRequestContext(RequestContext& ctx) const;
    const RequestContext& GetRequestContext() const;

    template<class Body, class Allocator>
    http::status HandleApiRequest(http::request<Body, http::basic_fields<Allocator>>&& request, std::string& response)
    {
        std::lock_guard<std::mutex> guard(mutex_);

        std::string path(request.target().data(), request.target().length());
        std::string token(request["Authorization"]);
        auto& usersData = CUsersData::Instance();

        if (request.method() == http::verb::post && path == "/api/login") {
            return LoginUser(request.body(), response);
        }

        // is user logged
        if (token == "" || token == "undefined") return http::status::unauthorized;
        if (!usersData.VerifyToken(token)) return http::status::unauthorized;

        // is route exists
        if (route_methods_.count(path) == 0) {
            response = "Method not found: " + path;
            return http::status::bad_request;
        }

        // TODO: perform this check while OPTIONS request to return correct 'Access-Control-Allow-Methods' header
        // is http method allowed on this route
        if (!MethodAllowed(request.method(), path)) {
            return http::status::method_not_allowed;
        }

        // is user autorized to call method
        if (!usersData.CheckAccess(token, path)) return http::status::forbidden;

        // set context
        request_context_.token = token;
        request_context_.route = path;
        request_context_.body = request.body();

        if (ExecuteRouteMethod(response)) {
            return http::status::ok;
        } else {
            return http::status::internal_server_error;
        }
    }

private:

    struct route_method
    {
        std::function<bool(std::string&)> func;
        std::vector<http::verb> allowed_methods;
    };

    CAPIHandler();
	~CAPIHandler();

	CAPIHandler(CAPIHandler const&) = delete;
	CAPIHandler& operator= (CAPIHandler const&) = delete;

    bool ExecuteRouteMethod(std::string& response) const;
    bool MethodAllowed(http::verb method, std::string& route) const;
    http::status LoginUser(const std::string& body, std::string& response) const;

    std::map<std::string, route_method> route_methods_;
    RequestContext request_context_;
    mutable std::mutex mutex_;
};

