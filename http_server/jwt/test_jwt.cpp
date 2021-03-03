#include <boost/test/unit_test.hpp>

#include "json_web_tokens.h"

BOOST_AUTO_TEST_CASE( jwt_usage_test )
{
    std::string token_data("123");
    std::string token;

    jwtoken::CreateToken(token_data, token);

    BOOST_TEST( jwtoken::Verify(token) );
}