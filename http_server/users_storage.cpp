#include <regex>

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
    usr.login = "admin";
    usr.pass = "pass";
    usr.isAdmin = true;
    usr.routes.push_back({ "/api/*" });

    users_.push_back(usr);

    usr.name = "Gosha";
    usr.login = "user";
    usr.pass = "user";
    usr.isAdmin = false;

    usr.routes.clear();
    usr.routes.push_back({ "/api/settings" });
    usr.routes.push_back({ "/api/login" });
    usr.routes.push_back({ "/api/logout" });
    usr.routes.push_back({ "/api/info" });

    users_.push_back(usr);
}

bool CUsersData::AuthenticateUser(const std::string& name, const std::string& pass, std::string& token)
{
    std::lock_guard<std::mutex> guard(mutex_);

    return true;
}

void CUsersData::DisproveUser(const std::string& name, const std::string& pass)
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
