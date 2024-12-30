#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>

#include <lib/cxxopts.hpp>

#include "event.hpp"
#include "parsing.hpp"
#include "requests.hpp"

#define VERSION_STRING "github-activity version 0.1.0"

int main(const int argc, const char* argv[]) {
    cxxopts::Options options("github-activity");

    options.add_options()
        ("username", "The Github username to fetch information for.", cxxopts::value<std::string>())
        ("v,version", "Display version information.", cxxopts::value<bool>()->default_value("false"))
        ("h,help", "Show this message", cxxopts::value<bool>()->default_value("false"));
    options.parse_positional({"username"});
    options.positional_help("<username>");

    try {
        auto shell_options = options.parse(argc, argv);

        if (shell_options.count("help")) {
            std::cout << options.help() << std::endl;
            return EXIT_SUCCESS;
        }
        if (shell_options.count("version")) {
            std::cout << VERSION_STRING << std::endl;
            return EXIT_SUCCESS;
        }

        auto username = shell_options["username"].as<std::string>();
        const std::string endpoint = format_api_endpoint(username);

        const std::string response_data = get_json_response(endpoint);
        const std::vector<Event> events = parse_json_response(response_data);

        for (const Event& event : events)
            std::cout << "- " << event.to_str() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
