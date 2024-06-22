#!/bin/bash
GREEN="\e[1;32m"
RESET="\e[0m"

# Function to check disk space
check_disk_space() {
    local required_space_gb=3
    local available_space_gb=$(df / | tail -1 | awk '{print $4/1024/1024}')

    if (( $(echo "$available_space_gb < $required_space_gb" | bc -l) )); then
        echo "You need at least 3 GB of disk space to install Nix. Available space: ${available_space_gb} GB"
        exit 1
    fi
}

remove_qrookie() {
    rm -rf /Applications/QRookie.app
    if ! command -v nix &> /dev/null
    then
        nix profile remove QRookie
    fi
}

if [[ "$1" == "uninstall" ]]; then

    remove_qrookie
    echo "Uninstallation completed!"

    exit 0
fi

# Check if Nix is installed
if ! command -v nix &> /dev/null
then
    echo "Nix is not installed. To install execute the following command:"
    # echo in green
    printf "${GREEN}export NIX_FIRST_BUILD_UID=306 && sh <(curl -L https://nixos.org/nix/install) --daemon-user-count 4 --yes${RESET}\n"

    echo "then run this script again."

    exit 1
fi

# Remove any existing QRookie.app in /Applications
echo "Removing existing QRookie.app from /Applications..."
remove_qrookie

# Install QRookie.app using Nix
echo "Installing QRookie.app..."

export NIXPKGS_ALLOW_UNSUPPORTED_SYSTEM=1
nix profile install --impure github:glaumar:nur#qrookie \
--extra-experimental-features nix-command \
--extra-experimental-features flakes

# Create symbolic link
echo "Creating symbolic link..."
ln -s ~/.nix-profile/Applications/QRookie.app /Applications/QRookie.app

echo "Installation completed!"

open /Applications/QRookie.app