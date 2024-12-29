#include <curl/curl.h>
#include <iostream>
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <cstring>
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
 * @brief Represents a Github event.
 */
struct Event {
    std::string type;
    std::string time;
    std::string repo_name;
    std::optional<int> issue_number;
    std::optional<int> pr_number;
    std::optional<int> commit_count; // # of commits in push
    std::optional<std::string> action;       // action taken (create, edit, delete, etc.)
    std::optional<std::string> assignee;     // username of user assigned to issue/PR
    std::optional<std::string> label;        // label added to/removed from issue/PR
    std::optional<std::string> collaborator; // username of collaborator added to repo
    std::optional<std::string> pr_title;
    std::optional<std::vector<std::string>> requested_reviewers;  // usernames

    /**
     * @brief Returns a human-readable string representation of the Event.
     *
     * @return  A string containing event information.
     */
    std::string to_str() const {
        const char* type_c_str = type.c_str();
        // NOTE: action_phrase should NEVER end with whitespace, EVER
        std::string action_phrase;

        std::function<std::string()> handle_issue_comment_event = [&]() -> std::string {
            std::string action_phrase;
            const std::string suffix = "a comment on issue #" + std::to_string(issue_number.value()) + " in";

            std::unordered_map<std::string, std::function<std::string()>> action_verb_map = {
                {"created", [&]() { return "Left " + suffix; }},
                {"edited", [&]() { return "Edited " + suffix; }},
                {"deleted", [&]() { return "Deleted " + suffix; }}
            };

            if (action_verb_map.find(*action) != action_verb_map.end()) {
                action_phrase = action_verb_map[*action]();
            }

            return action_phrase;
        };

        std::function<std::string()> handle_issue_event = [&]() -> std::string {
            std::string action_phrase;
            const std::string suffix = "issue #" + std::to_string(issue_number.value()) + " in";

            std::unordered_map<std::string, std::function<std::string()>> action_verb_map = {
                {"opened", [&]() { return "Opened " + suffix; }},
                {"edited", [&]() { return "Edited " + suffix; }},
                {"closed", [&]() { return "Closed " + suffix; }},
                {"reopened", [&]() { return "Reopened " + suffix; }},
            };

            if (assignee.has_value()) {
                if (*action == "assigned") {
                    action_phrase = "Assigned " + assignee.value() + " to " + suffix;
                } else if (*action == "unassigned") {
                    action_phrase = "Unassigned " + assignee.value() + " to " + suffix;
                }
            } else if (label.has_value()) {
                if (*action == "labeled") {
                    action_phrase = "Labeled issue #" + std::to_string(issue_number.value()) + " as \"" + label.value() + "\" in";
                }
            } else if (action_verb_map.find(*action) != action_verb_map.end()) {
                action_phrase = action_verb_map[*action]();
            }

            return action_phrase;
        };

        std::function<std::string()> handle_pr_event = [&]() -> std::string {
            std::string action_phrase;
            const std::string suffix = "PR #" + std::to_string(pr_number.value()) + " \"" + pr_title.value() + "\" in";

            /**
            * @brief Takes a vector of usernames and constructs a pluralized string.
            *
            * This lambda handles both singular and plural cases. If the vector contains more than one username,
            * it returns a pluralized string. For vectors with <=1 usernames, an empty string is returned.
            *
            * @param reviewers  Vector containing the requested reviewers' usernames.
            * @return           Pluralized string.
            */
            auto requested_reviewers_pluralize = [&]() -> std::string {
                // pluralize if more than one requested reviewer
                if (requested_reviewers->size() > 1) {
                    return (requested_reviewers->size() > 2)
                        ? ", " + requested_reviewers.value()[1] + " and others"
                        : " and " + requested_reviewers.value()[1];
                }

                // otherwise return empty string
                return "";
            };

            // don't require special attention, simple formula
            std::unordered_map<std::string, std::function<std::string()>> action_verb_map = {
                {"opened", [&]() { return "Opened " + suffix; }},
                {"edited", [&]() { return "Edited " + suffix; }},
                {"closed", [&]() { return "Closed " + suffix; }},
                {"reopened", [&]() { return "Reopened " + suffix; }},
                {"synchronize", [&]() { return "Updated " + suffix; }},
            };

            // special cases
            if (label.has_value()) {
                if (std::strcmp(action.value().c_str(), "labeled") == 0) {
                    // user added a label to a PR
                    action_phrase = "Labeled PR #" + std::to_string(pr_number.value()) + " \"" + label.value() + "\" in";
                } else if (std::strcmp(action.value().c_str(), "unlabeled") == 0) {
                    // user removed a label from a PR
                    action_phrase = "Removed label \"" + label.value() + "\" from PR #" + std::to_string(pr_number.value()) + " in";
                }
            } else if (assignee.has_value()) {
                if (std::strcmp(action.value().c_str(), "assigned") == 0) {
                    // user assigned someone to a PR
                    action_phrase = "Assigned " + assignee.value() + " to " + suffix;
                } else if (std::strcmp(action.value().c_str(), "edited") == 0) {
                    // user unassigned someone from a PR
                    action_phrase = "Unassigned " + assignee.value() + " from " + suffix;
                }
            } else if (requested_reviewers.has_value() && !requested_reviewers.value().empty()) {
                if (std::strcmp(action.value().c_str(), "review_requested") == 0) {
                    // user requested a PR review from someone
                    action_phrase = "Requested a review of PR #" + 
                        std::to_string(pr_number.value()) + " \"" + 
                        pr_title.value() + "\" from " + 
                        requested_reviewers.value()[0] +
                        requested_reviewers_pluralize() + " in";
                } else if (std::strcmp(action.value().c_str(), "review_request_removed") == 0) {
                    // user removed a PR review request
                    action_phrase = "Rescinded a review request for " +
                        requested_reviewers.value()[0] +
                        requested_reviewers_pluralize() + " on " + suffix;
                }
            } else {
                if (action_verb_map.find(*action) != action_verb_map.end()) {
                    action_phrase = action_verb_map[*action]();
                }
            }

            return action_phrase;
        };

        std::function<std::string()> handle_pr_review_thread_event = [&]() -> std::string {
            if (std::strcmp(action.value().c_str(), "resolved") == 0) {
                // user marked a PR review thread as resolved
                return "Marked a review comment thread as resolved in PR #" + std::to_string(pr_number.value()) + " \"" + pr_title.value() + "\" in";
            } else if (std::strcmp(action.value().c_str(), "unresolved") == 0) {
                // user marked a PR review thread as unresolved
                return "Marked a review comment thread as unresolved in PR #" + std::to_string(pr_number.value()) + " \"" + pr_title.value() + "\" in";
            } else {
                // something's gone horribly wrong
                return "Whoops!";
            }
        };

        std::function<std::string()> handle_pr_review_comment_event = [&]() -> std::string {
            return "Left a comment in a review of PR #" + std::to_string(pr_number.value()) + " \"" + pr_title.value() + "\" in";
        };

        std::function<std::string()> handle_pr_review_event = [&]() -> std::string {
            return "Reviewed PR #" + std::to_string(pr_number.value()) + " \"" + pr_title.value() + "\" in";
        };

        if (pr_title.has_value()) {
            std::unordered_map<std::string, std::function<std::string()>> pr_event_handle_map = {
                {"PullRequestEvent", handle_pr_event},
                {"PullRequestReviewThreadEvent", handle_pr_review_thread_event},
                {"PullRequestReviewCommentEvent", handle_pr_review_comment_event},
                {"PullRequestReviewEvent", handle_pr_review_event}
            };

            action_phrase = pr_event_handle_map[type_c_str]();

        } else if (issue_number.has_value()) {
            std::unordered_map<std::string, std::function<std::string()>> issue_event_handle_map = {
                {"IssueCommentEvent", handle_issue_comment_event},
                {"IssuesEvent", handle_issue_event},
            };

            action_phrase = issue_event_handle_map[type_c_str]();

        } else if (collaborator.has_value()) {
            // user added a collaborator to a repo
            action_phrase = "Added " + collaborator.value() + " as a collaborator on";
            // NOTE: wasn't able to find other potential actions (aside from edited), look into this

        } else if (commit_count.has_value()) {
            auto pluralize_commit_count = [&]() -> std::string {
                return (commit_count.value() == 1 ? " commit" : " commits");
            };

            // user pushed commit(s)
            action_phrase = "Pushed " + std::to_string(commit_count.value()) + pluralize_commit_count() + " to";

        } else {
            std::unordered_map<std::string, std::string> atomic_event_verb_map = {
                {"CommitCommentEvent", "Left a commit comment on"},
                {"CreateEvent", "Created a new branch/tag in"},
                {"DeleteEvent", "Deleted a branch/event in"},
                {"ForkEvent", "Forked"},
                {"GollumEvent", "Created/updated the wiki for"},
                {"PublicEvent", "Made repo public:"},
                {"ReleaseEvent", "Published a new release of"},
                {"SponsorshipEvent", "Sponsorship listing created for"},
                {"WatchEvent", "Starred"}
            };

            if (atomic_event_verb_map.find(type_c_str) != atomic_event_verb_map.end()) {
                action_phrase = atomic_event_verb_map[type_c_str];
            } else {
                // something must've gone horribly wrong
                action_phrase = "Whoops!";
            }

        }

        return action_phrase + " " + repo_name;
    };
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
