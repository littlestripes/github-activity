#include <iostream>

#include <lib/json.hpp>

#include "event.hpp"
#include "parsing.hpp"

/**
 * @brief Takes a Github API JSON response and returns a vector of Events containing each event's data.
 *
 * @param response  The raw JSON response.
 * @return          A vector containing Events.
 */
std::vector<Event> parse_json_response(const std::string& response) {
    using json = nlohmann::json;

    std::vector<Event> events;

    if (json::accept(response)) {
        // valid JSON
        const json response_json = json::parse(response);

        if (response_json.contains("status") && response_json["status"] == "404") {
            // user not found
            std::cerr << "Error: user not found" << std::endl;
            return std::vector<Event>();  // return a blank vector
        } else {
            for (auto it : response_json) {
                Event new_event = {
                    it["type"],
                    it["created_at"],
                    it["repo"]["name"],
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    std::nullopt
                };

                // check for optional fields and set them if available
                if (it["payload"].contains("action")) {
                    new_event.action = it["payload"]["action"];
                    if (std::strcmp(new_event.action.value().c_str(), "assigned") == 0) {
                        new_event.assignee = it["payload"]["assignee"]["login"];
                    }
                }
                if (it["payload"].contains("issue")) {
                    new_event.issue_number = it["payload"]["issue"]["number"];
                }
                if (it["payload"].contains("member")) {
                    new_event.collaborator = it["payload"]["member"]["login"];
                }
                if (it["payload"].contains("label")) {
                    new_event.label = it["payload"]["label"]["name"];
                }
                if (it["payload"].contains("pull_request")) {
                    new_event.pr_title = it["payload"]["pull_request"]["title"];
                    new_event.pr_number = it["payload"]["pull_request"]["number"];
                    if (it["payload"]["pull_request"].contains("requested_reviewers")) {
                        std::vector<std::string> usernames;
                        const auto& reviewer_objects_json = it["payload"]["pull_request"]["requested_reviewers"];

                        // small performance boost
                        usernames.reserve(reviewer_objects_json.size());

                        for (const auto& user : reviewer_objects_json) {
                            usernames.push_back(user["login"]);
                        }
                    }
                }
                if (it["payload"].contains("commits")) {
                    new_event.commit_count = it["payload"]["commits"].size();
                }

                events.push_back(new_event);
            }
        }
    } else {
        // invalid json, return a blank vector
        std::cerr << "Error: invalid JSON response" << std::endl;
        return std::vector<Event>();
    }

    return events;
}
