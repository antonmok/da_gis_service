#pragma once

#include <mutex>
#include <vector>
#include <string>

class CUsersData
{

public:

    struct UserData {
        bool isAdmin;
        bool isUser;
        bool firstLogin;
        unsigned id;
        std::string name;
        std::string position;
        std::string login;
        std::string pass;
        std::vector<std::string> routes;
        std::vector<std::string> tokens;
    };

    static CUsersData& Instance();

    // On success returns true and fills token 
    bool AuthenticateUser(const std::string& name, const std::string& pass, std::string& token);
    void DisproveUser(const std::string& name, const std::string& pass);

    bool CheckAccess(const std::string& token) const;
    bool CheckToken(const std::string& token) const;

    void GetUserData(const std::string& token) const;
    void GetUserData(unsigned id) const;

    // TODO: store users in bd
    void AddUser(const UserData&);
    void DeleteUser();

private:

    CUsersData() /*{}*/;
	~CUsersData() {};

	CUsersData(CUsersData const&) = delete;
	CUsersData& operator= (CUsersData const&) = delete;

    mutable std::mutex mutex_;
    std::vector<UserData> users_;

};
