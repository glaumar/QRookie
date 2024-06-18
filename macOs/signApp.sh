#!/bin/bash
source ./projectConfig.sh

WORKING_DIR="$(pwd)"
BUILD_DIR="$(pwd)/build_${ARCH}"
APP_PATH="QRookie.app"

./buildMacOs.sh

sign_item() {
    local ITEM_PATH=$1
    echo "Signing ${ITEM_PATH} with identity '${DEVELOPER_ID}'"
    codesign --force --options runtime --sign "${DEVELOPER_ID}" "${ITEM_PATH}"
}

echo "Packaging the app bundle... Dir = ${BUILD_DIR}"
cd "${BUILD_DIR}"

mkdir -p "${APP_PATH}/Contents/Resources"
cp -r "/opt/homebrew/Cellar/qt/6.7.0_1/share/qt/qml" "${APP_PATH}/Contents/Resources/"

macdeployqt "${APP_PATH}"

echo "Signing all"
find "$APP_PATH" -type f \( -name "*.dylib" -o -name "*.so" -o -name "*.framework" \) -print0 | while IFS= read -r -d '' file; do
    sign_item "$file"
done

echo "Signing executables..."
sign_item "${APP_PATH}/Contents/MacOS/QRookie"
sign_item "${APP_PATH}/Contents/Resources/adb"
sign_item "${APP_PATH}/Contents/Resources/7za"
sign_item "${APP_PATH}/Contents/Resources/zipalign"
sign_item "${APP_PATH}/Contents/Resources/apksigner"
sign_item "${APP_PATH}/Contents/Resources/apktool"

echo "Signing the app bundle..."
sign_item "${APP_PATH}"./

DMG_DIR="./Dmg"
rm -rf "$DMG_DIR"
mkdir -p "$DMG_DIR"
cp -r "$APP_PATH" "$DMG_DIR/"

APP_NAME="QRookie"
DMG_NAME="$WORKING_DIR/${APP_NAME}_${ARCH}.dmg"
hdiutil create -volname "$APP_NAME" -srcfolder "$DMG_DIR" -ov -format UDBZ "$DMG_NAME"
rm -rf "$DMG_DIR"

sign_item "$DMG_NAME"

xcrun notarytool submit "$DMG_NAME" \
    --keychain-profile "$ACCOUNT_PROFILE" \
    --wait

echo "Signing completed."

echo -e "\e[1;32mVerifying the app bundle...\e[0m"

spctl --assess --type exec -vv "${APP_PATH}"
codesign --verify --deep --strict --verbose=2 "${APP_PATH}"

