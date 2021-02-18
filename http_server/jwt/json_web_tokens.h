#pragma once

namespace jwtoken {

void decode(const std::string& token, std::string& output);

void create_token(const std::string& data, std::string& token);

}
