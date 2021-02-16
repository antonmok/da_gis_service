

#include "users_storage.h"

CUsersData& CUsersData::Instance()
{
	static CUsersData singletonInstance;
	return singletonInstance;
}

CUsersData::CUsersData() 
{
    //users_.push_back({ });
}

bool CUsersData::AuthenticateUser(const std::string& name, const std::string& pass, std::string& token)
{
    return true;
}

void CUsersData::DisproveUser(const std::string& name, const std::string& pass)
{

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

}

void CUsersData::GetUserData(unsigned id) const
{

}

// TODO: store users in bd
void CUsersData::AddUser(const UserData&)
{

}

void CUsersData::DeleteUser()
{

}
