#pragma once

#include <boost/beast/core.hpp>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>

beast::error_code handle_api_request(const boost::string_view path, std::string& resp_str);
beast::error_code handle_api_post_request(const boost::string_view path, std::string& resp_str);
