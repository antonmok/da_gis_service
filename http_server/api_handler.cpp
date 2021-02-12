
#include "api_handler.h"

beast::error_code handle_api_request(const boost::string_view path, std::string& resp_str)
{
    resp_str = std::string("{\n  \"method\": \"GET\",\n  \"route\": \"") + path.data() + "\",\n  \"message\": \"Hello\"\n}";

    return beast::errc::make_error_code(beast::errc::success);
}

boost::beast::error_code handle_api_post_request(const boost::string_view path, std::string& resp_str)
{
    resp_str = std::string("{\n  \"method\": \"POST\",\n  \"route\": \"") + path.data() + "\",\n  \"message\": \"Hello\"\n}";

    return beast::errc::make_error_code(beast::errc::success);
}

