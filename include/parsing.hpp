#ifndef PARSING_HPP
#define PARSING_HPP

#include <string>
#include <vector>

#include "event.hpp"

std::vector<Event> parse_json_response(const std::string& response);

#endif  // PARSING_HPP
