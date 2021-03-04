#include <regex>
#include <iostream>
#include <boost/json/src.hpp> // for header only version

#include "users_storage.h"
#include "jwt/json_web_tokens.h"

CUsersData& CUsersData::Instance()
{
	static CUsersData singletonInstance;
	return singletonInstance;
}

CUsersData::CUsersData() 
{
    UserData usr;

    usr.name = "Ivan Ivanovich";
    usr.position = "Developer";
    usr.credentials.username = "admin";
    usr.credentials.password = "123";
    usr.isAdmin = true;
    //usr.routes.push_back({ "/api/*" }); // TODO: use regexp
    usr.routes.push_back({ "/api/settings" });
    usr.routes.push_back({ "/api/logout" });
    usr.routes.push_back({ "/api/info" });
    usr.routes.push_back({ "/api/secret" });

    users_.push_back(usr);

    usr.name = "Gosha";
    usr.credentials.username = "user";
    usr.credentials.password = "456";
    usr.isAdmin = false;

    usr.routes.clear();
    usr.routes.push_back({ "/api/settings" });
    usr.routes.push_back({ "/api/logout" });
    usr.routes.push_back({ "/api/info" });

    users_.push_back(usr);
}

bool CUsersData::AuthenticateUser(const std::string& username, const std::string& password, std::string& token)
{
    std::lock_guard<std::mutex> guard(mutex_);

    for (auto& user : users_) {

        if (user.credentials.username == username) {
            if (user.credentials.password == password) {
                // create token
                jwtoken::CreateToken(username, token);
                // assign token to user
                user.tokens.push_back(token);
                return true;
            }
        }
    }

    return false;
}

bool CUsersData::AuthenticateUser(const Credentials& creds, std::string& token)
{
    return AuthenticateUser(creds.username, creds.password, token);
}

void CUsersData::DisproveUser(const std::string& token)
{
    std::lock_guard<std::mutex> guard(mutex_);

    for (auto user_ptr = users_.begin(); user_ptr != users_.end(); ++user_ptr) {
        for (auto token_ptr = user_ptr->tokens.begin(); token_ptr != user_ptr->tokens.end(); ++token_ptr) {
            if (*token_ptr == token) {
                user_ptr->tokens.erase(token_ptr);
                return;
            }
        }
    }
}

bool CUsersData::CheckAccess(const std::string& token, const std::string& requesting_route) const
{
    const auto& routes = GetUserData(token).routes;

    for (const auto& route : routes) {
        // TODO: use regex
        if (route == requesting_route) {
            return true;
        }
    }

    return false;
}

bool CUsersData::VerifyToken(const std::string& token)
{
    if (!TokenExist(token)) return false;

    bool valid = jwtoken::Verify(token);
    if (!valid) DisproveUser(token);

    return valid;
}

CUsersData::UserData CUsersData::GetUserData(const std::string& token) const
{
    std::lock_guard<std::mutex> guard(mutex_);

    for (const auto& user : users_) {
        if (std::find_if(user.tokens.begin(),
                        user.tokens.end(),
                        [token](const std::string& in) {
                            return in.find(token)!= std::string::npos;
                        }) != user.tokens.end())
        {
            return user;
        }
    }

    return UserData {};
}

bool CUsersData::TokenExist(const std::string& token) const
{
    std::lock_guard<std::mutex> guard(mutex_);

    for (const auto& user : users_) {
        if (std::find_if(user.tokens.begin(),
                        user.tokens.end(),
                        [token](const std::string& in) {
                            return in.find(token)!= std::string::npos;
                        }) != user.tokens.end())
        {
            return true;
        }
    }

    return false;
}

void CUsersData::AddUser(const UserData&)
{
    std::lock_guard<std::mutex> guard(mutex_);

}

void CUsersData::DeleteUser()
{
    std::lock_guard<std::mutex> guard(mutex_);

}
