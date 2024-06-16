# Description: This script is used to build the project on MacOS
PROJECT_DIR="$(pwd)/.."
BUILD_DIR="$(pwd)/build"
INSTALL_DIR="$(pwd)/dependencies/install"

# Build dependencies
./installDependencies.sh

# Build the project
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
rm -rf QRookie.app
cmake ../../
make

# Run
# open QRookie.app