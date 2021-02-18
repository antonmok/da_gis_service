#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

http::status handle_api_request(const std::string& path, std::string& resp_str);
http::status handle_api_post_request(const std::string& path, const std::string& body, std::string& resp_str);
