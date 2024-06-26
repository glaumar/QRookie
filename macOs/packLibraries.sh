source ./projectConfig.sh

WORKING_DIR="$(pwd)"
BUILD_DIR="$(pwd)/build_${ARCH_NAME}"

cd "${BUILD_DIR}"

echo "Copying qml to bundle..."
BREW_QML_VERSION=$(ls "$HOMEBREW_CELLAR/qt" | sort -r | head -n 1)
SRC_DIR="${HOMEBREW_CELLAR}/qt/${BREW_QML_VERSION}/share/qt/qml/"
DEST_DIR="${APP_PATH}/Contents/Resources/Qml/"
FOLDERS=("Qt" "QtCore" "QtNetwork" "QtQml" "QtQuick")
FOLDERS=("QtQuick")

mkdir -p "${DEST_DIR}"
for FOLDER in "${FOLDERS[@]}"; do
    SRC_PATH="${SRC_DIR}${FOLDER}"
    if [ -d "${SRC_PATH}" ]; then
        cp -R "${SRC_PATH}" "${DEST_DIR}"
        echo "Copied ${SRC_PATH} to ${DEST_DIR}"
    else
        echo "Directory ${SRC_PATH} does not exist"
    fi
done

echo "Packaging the app bundle with macdeployqt..."
macdeployqt "$APP_PATH" -codesign="$DEVELOPER_ID" -hardened-runtime -timestamp -sign-for-notarization="$DEVELOPER_ID"
