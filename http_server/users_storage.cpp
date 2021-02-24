#include <regex>

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
    usr.routes.push_back({ "/api/*" });

    users_.push_back(usr);

    usr.name = "Gosha";
    usr.credentials.username = "user";
    usr.credentials.password = "ololo";
    usr.isAdmin = false;

    usr.routes.clear();
    usr.routes.push_back({ "/api/settings" });
    usr.routes.push_back({ "/api/login" });
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

void CUsersData::DisproveUser(const std::string& username)
{
    std::lock_guard<std::mutex> guard(mutex_);


}

bool CUsersData::CheckAccess(const std::string& token) const
{
    return true;
}

bool CUsersData::CheckToken(const std::string& token) const
{
    return true;
}

void CUsersData::GetUserData(const std::string& token) const
{
    std::lock_guard<std::mutex> guard(mutex_);

}

void CUsersData::GetUserData(unsigned id) const
{
    std::lock_guard<std::mutex> guard(mutex_);

}

// TODO: store users in bd
void CUsersData::AddUser(const UserData&)
{
    std::lock_guard<std::mutex> guard(mutex_);

}

void CUsersData::DeleteUser()
{
    std::lock_guard<std::mutex> guard(mutex_);

}
