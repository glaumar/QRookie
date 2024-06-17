#!/bin/bash
ARCH=$(arch)
WORKING_DIR="$(pwd)"
BUILD_DIR="$(pwd)/build_${ARCH}"
APP_PATH="QRookie.app"
TEAM_ID="DU6489YN3U"
DEVELOPER_ID="Developer ID Application: Victor Cleber Laureano (${TEAM_ID})"

./buildMacOs.sh

sign_item() {
    local ITEM_PATH=$1
    echo "Signing ${ITEM_PATH} with identity '${DEVELOPER_ID}'"
    codesign --deep --force --options runtime --verbose --sign "${DEVELOPER_ID}" "${ITEM_PATH}"
}

echo "Packaging the app bundle... Dir = ${BUILD_DIR}"
cd "${BUILD_DIR}"

mkdir -p "${APP_PATH}/Contents/Resources"
cp -r "/opt/homebrew/Cellar/qt/6.7.0_1/share/qt/qml" "${APP_PATH}/Contents/Resources/"

# macdeployqt "${APP_PATH}"

echo "Signing main executable..."
sign_item "${APP_PATH}/Contents/MacOS/QRookie"

echo "Signing all"
find "$APP_PATH" -type f \( -name "*.dylib" -o -name "*.so" -o -name "*.framework" -o -name "*.app" \) -print0 | while IFS= read -r -d '' file; do
    sign_item "$file"
done

sign_item "${APP_PATH}/Contents/Resources/adb"
sign_item "${APP_PATH}/Contents/Resources/7za"
sign_item "${APP_PATH}/Contents/Resources/zipalign"
sign_item "${APP_PATH}/Contents/Resources/apksigner"
sign_item "${APP_PATH}/Contents/Resources/apktool"

echo "Signing the app bundle..."
sign_item "${APP_PATH}"

echo "Verifying the app bundle..."
spctl --assess --type exec -vv "${APP_PATH}"
codesign --verify --deep --strict --verbose=2 "${APP_PATH}"

DMG_DIR="./Dmg"
rm -rf "$DMG_DIR"
mkdir -p "$DMG_DIR"
cp -r "$APP_PATH" "$DMG_DIR/"

APP_NAME="QRookie"
DMG_NAME="$WORKING_DIR/${APP_NAME}_${ARCH}.dmg"
hdiutil create -volname "$APP_NAME" -srcfolder "$DMG_DIR" -ov -format UDBZ "$DMG_NAME"
rm -rf "$DMG_DIR"

sign_item "$DMG_NAME"

echo "Signing completed."