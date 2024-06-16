#!/bin/bash

BUILD_DIR="$(pwd)/build"
APP_PATH="./QRookie.app"
TEAM_ID="DU6489YN3U"
DEVELOPER_ID="Developer ID Application: Victor Cleber Laureano (${TEAM_ID})"

./buildMacOs.sh

# Função para assinar um item
sign_item() {
    local ITEM_PATH=$1
    echo "Signing ${ITEM_PATH} with identity '${DEVELOPER_ID}'"
    codesign --deep --force --verify --verbose --sign "${DEVELOPER_ID}" "${ITEM_PATH}"
}

# Empacotamento do app bundle
echo "Packaging the app bundle... Dir = ${BUILD_DIR}"
cd "${BUILD_DIR}"
macdeployqt "${APP_PATH}"

# Assinar o executável principal
echo "Signing main executable..."
sign_item "${APP_PATH}/Contents/MacOS/QRookie"

# Assinar todos os frameworks
echo "Signing all frameworks..."
for FRAMEWORK in "${APP_PATH}/Contents/Frameworks/"*; do
    if [ -d "$FRAMEWORK" ]; then
        sign_item "$FRAMEWORK"
    fi
done

# Assinar todas as bibliotecas
echo "Signing all libraries..."
for LIBRARY in "${APP_PATH}/Contents/Frameworks/"*.dylib; do
    if [ -f "$LIBRARY" ]; then
        sign_item "$LIBRARY"
    fi
done

# Assinar o app bundle
echo "Signing the app bundle..."
sign_item "${APP_PATH}"

# Verificar a assinatura
echo "Verifying the app bundle..."
spctl --assess --type exec -vv "${APP_PATH}"
codesign --verify --deep --strict --verbose=2 "${APP_PATH}"

echo "Signing completed."