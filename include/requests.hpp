#ifndef REQUESTS_HPP
#define REQUESTS_HPP

#include <cstdlib>
#include <string>

/**
 * @brief Builds and sends a GET request to the given API endpoint and returns JSON response data.
 *
 * @param endpoint  The API endpoint to make a request to.
 * @return          The response body (in JSON);
 */
std::string get_json_response(const std::string& endpoint);

#endif  // REQUESTS_HPP
