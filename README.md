# github-activity

Small command line utility to check a Github user's recent activity.

Look [here](https://roadmap.sh/projects/github-user-activity) for project guidelines.

## Build
### Dependencies
Fedora: `sudo dnf install gcc-c++ libcurl-devel pkg-config make`

Ubuntu/Debian: `sudo apt install g++ libcurl4-openssl-dev pkg-config make`

Arch: `sudo pacman -S gcc curl pkg-config make`

macOS: `brew install gcc curl pkg-config make`

`make`

### Generate `compile_commands.json`
`bear -- make`
