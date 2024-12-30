#include <cstdlib>
#include <string>
#include <iostream>

#include <curl/curl.h>

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
