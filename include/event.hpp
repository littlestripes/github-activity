#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>
#include <optional>
#include <vector>

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

    std::string to_str() const;
};

#endif  // EVENT_HPP
