# Description: This script is used to build the project on MacOS
ARCH=$(arch)
PROJECT_DIR="$(pwd)/.."
BUILD_DIR="$(pwd)/build_${ARCH}"
INSTALL_DIR="$(pwd)/dependencies/install_${ARCH}"
APP_RESOURCES_DIR="$BUILD_DIR/QRookie.app/Contents/Resources"

# Build dependencies
./installDependencies.sh

# Build the project
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
rm -rf QRookie.app
cmake ../../
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
copy_binary "adb" "$APP_RESOURCES_DIR"
copy_binary "7za" "$APP_RESOURCES_DIR"

# Run
# open QRookie.app