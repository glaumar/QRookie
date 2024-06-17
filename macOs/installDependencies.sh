#!/bin/bash

# Definir diretórios de instalação
ARCH=$(arch)
WORKING_DIR="$(pwd)"
DEP_DIR="${WORKING_DIR}/dependencies"
BUILD_DIR="${DEP_DIR}/build_${ARCH}"
INSTALL_DIR="${DEP_DIR}/install_${ARCH}"

mkdir -p "${DEP_DIR}"
mkdir -p "${BUILD_DIR}"
mkdir -p "${INSTALL_DIR}"

install_dependency() {
    local REPO_URL=$1
    local VERSION=$2
    local REPO_NAME=$(basename "$REPO_URL" .git)
    local REPO_DIR="${DEP_DIR}/${REPO_NAME}"

    if [ ! -d "$REPO_DIR" ]; then
        echo "Clonando $REPO_NAME..."
        git clone "$REPO_URL" "$REPO_DIR"
    else
        echo "$REPO_NAME já está clonado."
    fi

    if [ ! -d "${INSTALL_DIR}/${REPO_NAME}" ]; then
        echo "Construindo e instalando $REPO_NAME..."

        cd "$REPO_DIR"
        git checkout "$VERSION"

        mkdir -p "${BUILD_DIR}/${REPO_NAME}"
        cd "${BUILD_DIR}/${REPO_NAME}"
        cmake "$REPO_DIR" -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}/${REPO_NAME}"
        make
        make install
        cd "$WORKING_DIR"
    else
        echo "$REPO_NAME já está instalado."
    fi
}

set_brew_path() {
    if [ "$(arch)" = "arm64" ]; then
        # Definir PATH para Homebrew em /opt/homebrew (Apple Silicon)
        export PATH="$PATH:/opt/homebrew/bin"
        echo "Using Homebrew for Apple Silicon (arm64) at /opt/homebrew/bin"
    else
        # Definir PATH para Homebrew em /usr/local (Intel)
        export PATH="$PATH:/usr/local/bin"
        echo "Using Homebrew for Intel (x86_64) at /usr/local/bin"
    fi
}


# Install Homebrew dependencies
echo "Instalando ferramentas via Homebrew..."
set_brew_path

echo -e "\033[34mUsing these binary paths:\033[0m"
commands=("cmake" "make" "brew" "7za" "adb" "macdeployqt" "codesign")
for cmd in "${commands[@]}"; do
    path=$(which $cmd)
    echo -e "\033[33m$cmd:\033[0m $path"
done

brew install cmake extra-cmake-modules qt vulkan-headers ninja vulkan-loader molten-vk pkg-config node glib python@3.12 p7zip


# Install cmake dependencies
install_dependency "https://github.com/danvratil/qcoro.git" v0.10.0
install_dependency "https://invent.kde.org/frameworks/kirigami.git" v6.2.1

echo "Todas as dependências foram instaladas com sucesso."