

#include <chrono>
#include "jwt-cpp/jwt.h"
#include "json_web_tokens.h"

namespace jwtoken {

void decode(const std::string& token, std::string& output) {

	auto decoded = jwt::decode(token);

    // todo do something
	for (auto& e : decoded.get_payload_claims()) {
		std::cout << e.first << " = " << e.second << std::endl;
    }
}

void create_token(const std::string& data, std::string& token) {

}

}
