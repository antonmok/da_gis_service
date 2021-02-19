#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

template<class Body, class Allocator>
http::status handle_api_request(http::request<Body, http::basic_fields<Allocator>>&& req, std::string& resp_str)
{
    std::string path(req.target().data(), req.target().length());

    // is user loged

    // parse body as json

    // is user autorithed to call method

    // get method to call using regex

    // call

    // form http response

    resp_str.append("{\n  \"method\": \"GET\",\n  \"route\": \"");
    resp_str.append(path);
    resp_str.append("\",\n  \"message\": \"Hello\"\n}");

    return http::status::ok;
}

