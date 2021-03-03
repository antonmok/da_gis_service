#include <boost/test/unit_test.hpp>

#include "users_storage.h"

BOOST_AUTO_TEST_CASE( user_false_auth_test )
{
    Credentials creds;
    std::string token;

    BOOST_TEST(false == CUsersData::Instance().AuthenticateUser(creds, token));
}