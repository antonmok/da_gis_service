#define BOOST_TEST_MODULE http_server test module
#include <boost/test/included/unit_test.hpp>

#include "api_handler.h"
#include "jwt/json_web_tokens.h"
#include "users/users_storage.h"

/***********************************************/
BOOST_AUTO_TEST_SUITE( login_test_suite )

BOOST_AUTO_TEST_CASE( login_ok_test )
{
    http::request<http::string_body> req;
    std::string response;
    JSToken tok;

    req.body() = "{\"username\":\"user\",\"password\":\"456\"}";
    req.method(http::verb::post);
    req.target("/api/login");

    BOOST_TEST(http::status::ok == CAPIHandler::Instance().HandleApiRequest(std::move(req), response));
    BOOST_TEST(JsonToStruct<JSToken>(response, tok));
    BOOST_TEST(jwtoken::Verify(tok.token));

}
BOOST_AUTO_TEST_CASE( login_bad_json_test )
{
    http::request<http::string_body> req;
    std::string response;

    req.body() = "{\"username\":bad json,\"password\":\"32131231\"}";
    req.method(http::verb::post);
    req.target("/api/login");

    BOOST_TEST(http::status::bad_request == CAPIHandler::Instance().HandleApiRequest(std::move(req), response));
    BOOST_TEST(response == "Invalid json");
}

BOOST_AUTO_TEST_CASE( login_bad_method_test )
{
    http::request<http::string_body> req;
    std::string response;
    JSToken tok;

    req.body() = "{\"username\":\"user\",\"password\":\"456\"}";
    req.method(http::verb::get);
    req.target("/api/login");

    BOOST_TEST(http::status::unauthorized == CAPIHandler::Instance().HandleApiRequest(std::move(req), response));
}

BOOST_AUTO_TEST_SUITE_END()


/***********************************************/
BOOST_AUTO_TEST_SUITE( no_access_test_suite )

BOOST_AUTO_TEST_CASE( login_bad_method_test )
{
    http::request<http::string_body> req;
    std::string response;
    JSToken tok;

    req.method(http::verb::get);
    req.set("Authorization", "bad token");
    req.target("/api/info");

    BOOST_TEST(http::status::unauthorized == CAPIHandler::Instance().HandleApiRequest(std::move(req), response));
}
//unauthorized
BOOST_AUTO_TEST_SUITE_END()