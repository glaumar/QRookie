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
    codesign --deep --force --verify --verbose --sign "${DEVELOPER_ID}" "${ITEM_PATH}"
}

echo "Packaging the app bundle... Dir = ${BUILD_DIR}"
cd "${BUILD_DIR}"

mkdir -p "${APP_PATH}/Contents/Resources"
cp -r "/opt/homebrew/Cellar/qt/6.7.0_1/share/qt/qml" "${APP_PATH}/Contents/Resources/"

macdeployqt "${APP_PATH}"

echo "Signing main executable..."
sign_item "${APP_PATH}/Contents/MacOS/QRookie"

echo "Signing all frameworks..."
for FRAMEWORK in "${APP_PATH}/Contents/Frameworks/"*; do
    if [ -d "$FRAMEWORK" ]; then
        sign_item "$FRAMEWORK"
    fi
done

sign_item "${APP_PATH}/Contents/Resources/adb"
sign_item "${APP_PATH}/Contents/Resources/7za"

echo "Signing the app bundle..."
sign_item "${APP_PATH}"

echo "Verifying the app bundle..."
spctl --assess --type exec -vv "${APP_PATH}"
codesign --verify --deep --strict --verbose=2 "${APP_PATH}"

rm -rf "$WORKING_DIR/QRookie_${ARCH}.app"
zip -r "$APP_PATH" "$WORKING_DIR/QRookie_${ARCH}.zip"

echo "Signing completed."