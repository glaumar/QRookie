#!/bin/bash
export ARCH=$(arch)
export OS_TARGET="12"
if [[ "$ARCH" == "arm64" ]]; then
    export ARCH_NAME="arm64"
    eval "$(/opt/homebrew/bin/brew shellenv)"
    echo "Apple Silicon (arm64) detected."
    echo "Using Homebrew for Apple Silicon (arm64) at /opt/homebrew/bin"
else
    export ARCH_NAME="x86_64"
    eval "$(/usr/local/bin/brew shellenv)"
    echo "Intel (x86_64) detected."
    echo "Using Homebrew for Intel (x86_64) at /usr/local/bin"
fi

# APPLE_ID Env Var
# APPLE_PASSWD Env Var
# DEVELOPER_NAME Env Var
# TEAM_ID Env Var
export ACCOUNT_PROFILE="qrookienotorytool"
export APP_BUNDLE="io.github.glaumar.QRookie"
export DEVELOPER_ID="Developer ID Application: ${DEVELOPER_NAME} (${TEAM_ID})"

# xcrun notarytool store-credentials "$ACCOUNT_PROFILE" \
#     --team-id "$TEAM_ID" \
#     --apple-id "$APPLE_ID" \
#     --password "$APPLE_PASSWD"