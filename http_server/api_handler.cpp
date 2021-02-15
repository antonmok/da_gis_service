
#include "api_handler.h"

beast::error_code handle_api_request(const boost::string_view path, std::string& resp_str)
{
    resp_str.append("{\n  \"method\": \"GET\",\n  \"route\": \"");
    resp_str.append(path.data(), path.size());
    resp_str.append("\",\n  \"message\": \"Hello\"\n}");

    return beast::errc::make_error_code(beast::errc::success);
}

boost::beast::error_code handle_api_post_request(const boost::string_view path, std::string& resp_str)
{
    resp_str.append("{\n  \"method\": \"POST\",\n  \"route\": \"");
    resp_str.append(path.data(), path.size());
    resp_str.append("\",\n  \"message\": \"Hello\"\n}");

    return beast::errc::make_error_code(beast::errc::success);
}

