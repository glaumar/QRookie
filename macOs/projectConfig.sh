#!/bin/bash
export ARCH=$(arch)
export OS_TARGET="12"
if [[ "$ARCH" == "arm64" ]]; then
    export ARCH_NAME="arm64"
    eval "$(/opt/homebrew/bin/brew shellenv)"
    echo "Apple Silicon (arm64) detected."
    echo "Using Homebrew for Apple Silicon (arm64) at /opt/homebrew/bin"
    export Qt6_DIR="/opt/homebrew/opt/qt/lib/cmake/Qt6"
else
    export ARCH_NAME="x86_64"
    eval "$(/usr/local/bin/brew shellenv)"
    echo "Intel (x86_64) detected."
    echo "Using Homebrew for Intel (x86_64) at /usr/local/bin"
    export Qt6_DIR="/usr/local/opt/qt/lib/cmake/Qt6"
fi

set_android_tools() {
    if [ ! -d "$ANDROID_SDK_DIR" ]; then
        echo -e "\033[31mError:\033[0m BuildTools directory not found in $BUILD_TOOLS_DIR, using default path."
        export ANDROID_SDK_DIR="$HOME/Library/Android/sdk"
        echo "Using Android SDK at $ANDROID_SDK_DIR"
    fi
    local BUILD_TOOLS_DIR="$ANDROID_SDK_DIR/build-tools"
    local BUILD_TOOLS_VERSION=$(ls "$BUILD_TOOLS_DIR" | sort -r | head -n 1)
    export BUILD_TOOLS_BIN="$BUILD_TOOLS_DIR/$BUILD_TOOLS_VERSION"
    echo -e "\033[34mUsing Android Build Tools at $BUILD_TOOLS_BIN\033[0m"
    export PATH="$PATH:$BUILD_TOOLS_BIN"
}

# APPLE_ID Env Var
# APPLE_PASSWD Env Var
# DEVELOPER_NAME Env Var
# TEAM_ID Env Var
export ACCOUNT_PROFILE="qrookienotorytool"
export APP_BUNDLE="io.github.glaumar.QRookie"
export DEVELOPER_ID="Developer ID Application: ${DEVELOPER_NAME} (${TEAM_ID})"
export APP_NAME="QRookie"
export APP_PATH="${APP_NAME}.app"

# xcrun notarytool store-credentials "$ACCOUNT_PROFILE" \
#     --team-id "$TEAM_ID" \
#     --apple-id "$APPLE_ID" \
#     --password "$APPLE_PASSWD"