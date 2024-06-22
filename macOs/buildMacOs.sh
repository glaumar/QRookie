# Description: This script is used to build the project on MacOS
source ./projectConfig.sh

PROJECT_DIR="$(pwd)/.."
BUILD_DIR="$(pwd)/build_${ARCH_NAME}"
INSTALL_DIR="$(pwd)/dependencies/install_${ARCH_NAME}"
APP_RESOURCES_DIR="$BUILD_DIR/QRookie.app/Contents/Resources"

# Build dependencies
./installDependencies.sh

# Build the project
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
rm -rf QRookie.app
cmake ../../  -DCMAKE_OSX_ARCHITECTURES=$ARCH_NAME -DCMAKE_OSX_DEPLOYMENT_TARGET=$OS_TARGET
make

copy_binary() {
    local BINARY_NAME=$1
    local DEST_DIR=$2
    local BINARY_PATH=$(which "$BINARY_NAME" | head -n 1)

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
copy_binary ${HOMEBREW_CELLAR}/p7zip/*/lib/p7zip/7za "$APP_RESOURCES_DIR"
# Need Android SDK configured
copy_binary "apksigner" "$APP_RESOURCES_DIR"
cp -r "$BUILD_TOOLS_BIN/lib" "$APP_RESOURCES_DIR"
copy_binary "zipalign" "$APP_RESOURCES_DIR"

# Need user brew install
copy_binary "apktool" "$APP_RESOURCES_DIR"

# Backup the local linked app bundle
rm -rf "${APP_NAME}_local.app"
cp -r "$APP_PATH" "${APP_NAME}_local.app"

# If --open is passed, run the app binary
if [[ "$1" == "--open" ]]; then
    ./QRookie.app/Contents/MacOS/QRookie
fi