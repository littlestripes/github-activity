#include <functional>
#include <unordered_map>
#include <cstring>

#include "event.hpp"

/**
* @brief Returns a human-readable string representation of the Event.
*
* @return  A string containing event information.
*/
std::string Event::to_str() const {
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
