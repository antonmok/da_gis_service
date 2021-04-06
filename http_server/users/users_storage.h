#pragma once

#include <mutex>
#include <vector>
#include <string>

#include "api/json_structs.hpp" // +boost/json.hpp

class CUsersData
{
public:

    struct UserData {
        bool isAdmin = false;
        bool isUser = true;
        bool firstLogin = false;
        unsigned id = 0;
        std::string name;
        std::string position;
        Credentials credentials;
        std::vector<std::string> routes;    // wildcards available
        std::vector<std::string> tokens;
        JSSettings settings;
    };

    static CUsersData& Instance();

    // On success returns true and fills token 
    bool AuthenticateUser(const std::string& username, const std::string& pass, std::string& token);
    bool AuthenticateUser(const Credentials& credentials, std::string& token);
    void DisproveUser(const std::string& token);

    bool CheckAccess(const std::string& token, const std::string& route) const;
    bool VerifyToken(const std::string& token);

    UserData GetUserData(const std::string& token) const;
    void GetUserData(unsigned id) const;

    // TODO: store users in bd (mongoDB)
    void AddUser(const UserData&);
    void DeleteUser();

private:

    CUsersData() /*{}*/;
	~CUsersData() {};

	CUsersData(CUsersData const&) = delete;
	CUsersData& operator= (CUsersData const&) = delete;

    bool TokenExist(const std::string& token) const;

    mutable std::mutex mutex_;
    std::vector<UserData> users_;

};
