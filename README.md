# github-activity

Small command line utility to check a Github user's recent activity.

## Build
### Dependencies
Fedora: `sudo dnf install gcc-c++ libcurl-devel fmt-devel pkg-config make`

Ubuntu/Debian: `sudo apt install g++ libcurl4-openssl-dev libfmt-dev pkg-config make`

Arch: `sudo pacman -S gcc curl fmt pkg-config make`

macOS: `brew install gcc curl fmt pkg-config make`

`make`

### Generate `compile_commands.json`
`bear -- make`
