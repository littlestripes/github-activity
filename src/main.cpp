#include <curl/curl.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fmt/format.h>

#include "json.hpp"
#include "cxxopts.hpp"

/**
 * @brief Callback that handles HTTP response data.
 *
 * @param contents  The HTTP response data.
 * @param size      The size of each data element (usually 1 byte).
 * @param nmemb     The number of data elements received.
 * @param userp     Points to the location where the response data will be stored.
 * @return          The number of bytes stored at userp.
 */
size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * @brief Builds and sends a GET request to the given API endpoint and returns JSON response data.
 *
 * @param endpoint  The API endpoint to make a request to.
 */
std::string get_json_response(const std::string& endpoint) {
    CURL* curl;
    CURLcode response;
    std::string read_buffer;
    
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "accept: application/vnd.github+json");
        headers = curl_slist_append(headers, "User-Agent: curl/8.6.0");

        // headers
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        // API endpoint
        curl_easy_setopt(curl, CURLOPT_URL, endpoint.c_str());
        // set callback for writing response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        // pointer passed to write_callback, points to buffer for data to
        // be written to
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buffer);

        response = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if (response != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(response) << std::endl;
        }
    }

    return read_buffer;
}

/**
 * @brief Takes a Github username and returns the API endpoint for user activity.
 *
 * @param username  Github username.
 * @returns         The complete API endpoint.
 */
std::string format_api_endpoint(const std::string& username) {
    return fmt::format("https://api.github.com/users/{}/events", username);
}

int main(const int argc, const char* argv[]) {
    cxxopts::Options options("github-activity", "Fetch Github user activity");

    options.add_options()
        ("username", "The Github username to fetch information for.", cxxopts::value<std::string>())
        ("h,help", "Show this message", cxxopts::value<bool>()->default_value("false"));
    options.parse_positional({"username"});
    options.positional_help("<Github username>");

    try {
        auto shell_options = options.parse(argc, argv);

        if (shell_options.count("help")) {
            std::cout << options.help() << std::endl;
            return EXIT_SUCCESS;
        }

        auto username = shell_options["username"].as<std::string>();
        const std::string endpoint = format_api_endpoint(username);

        const std::string response_data = get_json_response(endpoint);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
