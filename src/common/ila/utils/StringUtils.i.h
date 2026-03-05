#pragma once
#include <string>

namespace ila::inline utils::string_utils {

std::string camelToSnakeWithoutM(std::string_view input);

std::string snakeToCamelWithM(std::string_view input);

} // namespace ila::inline utils::string_utils