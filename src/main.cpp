#include <curl/curl.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fmt/format.h>

#include "json.hpp"
#include "cxxopts.hpp"

#define VERSION_STRING "github-activity version 0.1.0"

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
 * @return          The response body (in JSON);
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
 * @brief Represents a Github event. Contains the event type, time occurred, and referenced repo.
 */
struct Event {
    std::string type;
    std::string time;
    std::string repo_name;

    /**
     * @brief Returns a human-readable string representation of the Event.
     *
     * @return A string containing the event data.
     */
    std::string to_str() const {
        // TODO: switch/case for different event types
        // Obviously we don't want to print "- WatchEvent ..."
        return "";
    }
};

/**
 * @brief Takes a Github API JSON response and returns a vector of Events containing each event's data.
 *
 * @param response  The raw JSON response.
 * @return          A vector containing Events.
 */
std::vector<Event> parse_json_response(const std::string& response) {
    return std::vector<Event>();
}

/**
 * @brief Takes a Github username and returns the API endpoint for user activity.
 *
 * @param username  Github username.
 * @return          The complete API endpoint.
 */
std::string format_api_endpoint(const std::string& username) {
    return fmt::format("https://api.github.com/users/{}/events", username);
}

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
            std::cout << options.help() << std::endl;
            return EXIT_SUCCESS;
        }

        auto username = shell_options["username"].as<std::string>();
        const std::string endpoint = format_api_endpoint(username);

        const std::string response_data = get_json_response(endpoint);
        const std::vector<Event> events = parse_json_response(response_data);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
