# Description: This script is used to build the project on MacOS
CURRENT_DIR="$(pwd)"

# Build dependencies
cd dependencies
./macOsInstall.sh

# Build the project
cd "$CURRENT_DIR"
mkdir -p build
cd build
cmake ..
make

# Run
open QRookie.app