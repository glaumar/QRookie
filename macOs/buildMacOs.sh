# Description: This script is used to build the project on MacOS
source ./projectConfig.sh

PROJECT_DIR="$(pwd)/.."
BUILD_DIR="$(pwd)/build_${ARCH}"
INSTALL_DIR="$(pwd)/dependencies/install_${ARCH}"
APP_RESOURCES_DIR="$BUILD_DIR/QRookie.app/Contents/Resources"

set_android_tools() {
    if [ ! -d "$ANDROID_SDK_DIR" ]; then
        echo -e "\033[31mError:\033[0m BuildTools directory not found in $BUILD_TOOLS_DIR, using default path."
        export ANDROID_SDK_DIR="$HOME/Library/Android/sdk"
        echo "Using Android SDK at $ANDROID_SDK_DIR"
    fi
    local BUILD_TOOLS_DIR="$ANDROID_SDK_DIR/build-tools"
    local BUILD_TOOLS_VERSION=$(ls "$BUILD_TOOLS_DIR" | sort -r | head -n 1)
    local BUILD_TOOLS_BIN="$BUILD_TOOLS_DIR/$BUILD_TOOLS_VERSION"
    echo -e "\033[34mUsing Android Build Tools at $BUILD_TOOLS_BIN\033[0m"
    export PATH="$PATH:$BUILD_TOOLS_BIN"
}
# Build dependencies
./installDependencies.sh

if [[ "$ARCH" == "arm64" ]]; then
    echo "Apple Silicon (arm64) detected."
    ARCH_NAME="arm64"
else
    echo "Intel (x86_64) detected."
    ARCH_NAME="x86_64"
fi

# Build the project
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
rm -rf QRookie.app
cmake ../../  -DCMAKE_OSX_ARCHITECTURES=$ARCH_NAME -DCMAKE_OSX_DEPLOYMENT_TARGET=$OS_TARGET
make

copy_binary() {
    local BINARY_NAME=$1
    local DEST_DIR=$2
    local BINARY_PATH=$(which "$BINARY_NAME")

    if [ -x "$BINARY_PATH" ]; then
        echo "Found $BINARY_NAME at $BINARY_PATH"
        cp "$BINARY_PATH" "$DEST_DIR"
        echo "Copied $BINARY_NAME to $DEST_DIR"
    else
        echo "Error: $BINARY_NAME not found"
    fi
}
set_android_tools

echo -e "\033[34mUsing these binary paths:\033[0m"
commands=("cmake" "make" "brew" "7za" "adb" "macdeployqt" "codesign" "qmake" "apktool" "zipalign" "apksigner")
for cmd in "${commands[@]}"; do
    path=$(which $cmd)
    if [ -L "$path" ]; then
        real_path=$(readlink "$path")
        echo -e "\033[33m$cmd:\033[0m $path -> $real_path"
    else
        echo -e "\033[33m$cmd:\033[0m $path"
    fi
done


copy_binary "adb" "$APP_RESOURCES_DIR"
copy_binary "7za" "$APP_RESOURCES_DIR"
copy_binary "apktool" "$APP_RESOURCES_DIR"
copy_binary "zipalign" "$APP_RESOURCES_DIR"
copy_binary "apksigner" "$APP_RESOURCES_DIR"

# Run
# open QRookie.app