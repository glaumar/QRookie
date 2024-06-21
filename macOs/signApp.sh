#!/bin/bash
source ./projectConfig.sh

WORKING_DIR="$(pwd)"
BUILD_DIR="$(pwd)/build_${ARCH}"
APP_PATH="QRookie.app"

# if --OnlySign is passed, not build and pack
if [[ "$1" == "--OnlySign" ]]; then
    echo "Only signing the app bundle..."
else
    ./buildMacOs.sh
    ./packLibraries.sh
fi

cd "${BUILD_DIR}"

printf "\e[1;32mRemove existing signature\e[0m\n"
find "$APP_PATH" \( -name "*.dylib" -o -name "*.framework" -o -name "*.app" \) -exec \
codesign --remove-signature {} \;

printf "\e[1;32mAssign all the libraries inside the app bundle\e[0m\n"
find "$APP_PATH" -type f -name "*.dylib" -exec \
codesign --deep --force --verify --verbose --timestamp --options runtime --sign "$DEVELOPER_ID" {} \;

printf "\e[1;32mAssign all the frameworks inside the app bundle\e[0m\n"
find "$APP_PATH" -type d -name "*.framework" -exec \
codesign --deep --force --verify --verbose --timestamp --options runtime --sign "$DEVELOPER_ID" {} \;

printf "\e[1;32mAssign all the frameworks binaries inside the app bundle\e[0m\n"
for framework in "$APP_PATH/Contents/Frameworks/"*.framework; do
    framework_name=$(basename "$framework" .framework)
    binary_path="$framework/$framework_name"
    if [ -f "$binary_path" ]; then
        codesign --force --verify --verbose --timestamp --options runtime --sign "$DEVELOPER_ID" "$binary_path"
    fi
done

printf "\e[1;32mAssign main binary\e[0m\n"
codesign --deep --force --verify --verbose --timestamp --options runtime --sign "$DEVELOPER_ID" "$APP_PATH/Contents/MacOS/QRookie"

printf "\e[1;32mAssign .app\e[0m\n"
codesign --deep --force --verify --verbose --timestamp --options runtime --sign "$DEVELOPER_ID" "$APP_PATH"


DMG_DIR="./Dmg"
rm -rf "$DMG_DIR"
mkdir -p "$DMG_DIR"
cp -r "$APP_PATH" "$DMG_DIR/"

APP_NAME="QRookie"
DMG_NAME="$WORKING_DIR/${APP_NAME}_${ARCH}.dmg"
hdiutil create -volname "$APP_NAME" -srcfolder "$DMG_DIR" -ov -format UDBZ "$DMG_NAME"
rm -rf "$DMG_DIR"

codesign --deep --force --verify --verbose --timestamp --options runtime --sign "$DEVELOPER_ID" "$DMG_NAME"

xcrun notarytool submit "$DMG_NAME" \
    --keychain-profile "$ACCOUNT_PROFILE" \
    --wait

echo "Signing completed."

printf "\e[1;32mVerifying the app bundle...\e[0m\n"
spctl --assess --type exec -vv "${APP_PATH}"
codesign --verify --deep --strict --verbose=2 "${APP_PATH}"

