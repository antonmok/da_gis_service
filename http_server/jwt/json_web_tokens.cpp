

#include <chrono>
#include "jwt-cpp/jwt.h"
#include "json_web_tokens.h"

#define SECRET_STRING "secret_string"
#define ISSUER	"BLABBER"

namespace jwtoken {

bool Verify(const std::string& token) {

	auto verify =
		jwt::verify().allow_algorithm(jwt::algorithm::hs256(SECRET_STRING)).with_issuer(ISSUER);

	auto decoded = jwt::decode(token);

	std::error_code ec;
	verify.verify(decoded, ec);

	if (ec) {
		std::cout << "token verification error: " << ec.message() << std::endl;
		return false;
	} else {
		return true;
	}
}

void CreateToken(const std::string& data, std::string& token) {
	// TODO: use token content signing, rsa, etc...

	token = jwt::create()
				.set_issuer(ISSUER)
				.set_type("JWT")
				.set_issued_at(std::chrono::system_clock::now())
				//.set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{24})
				.set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes{1})
				.set_payload_claim("username", jwt::claim(data))
				.sign(jwt::algorithm::hs256{SECRET_STRING});
}

} // namespace jwtoken end

