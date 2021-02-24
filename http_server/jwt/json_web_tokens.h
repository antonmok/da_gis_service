#pragma once

namespace jwtoken {

bool Verify(const std::string& token);

void CreateToken(const std::string& data, std::string& token);

}
