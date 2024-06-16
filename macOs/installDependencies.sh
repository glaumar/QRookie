#!/bin/bash

# Definir diretórios de instalação
WORKING_DIR="$(pwd)"
DEP_DIR="${WORKING_DIR}/dependencies"
BUILD_DIR="${DEP_DIR}/build"
INSTALL_DIR="${DEP_DIR}/install"

mkdir -p "${DEP_DIR}"
mkdir -p "${BUILD_DIR}"
mkdir -p "${INSTALL_DIR}"

# Função para clonar, construir e instalar dependências
install_dependency() {
    local REPO_URL=$1
    local VERSION=$2
    local REPO_NAME=$(basename "$REPO_URL" .git)
    local REPO_DIR="${DEP_DIR}/${REPO_NAME}"

    # Verificar se o repositório já foi clonado
    if [ ! -d "$REPO_DIR" ]; then
        echo "Clonando $REPO_NAME..."
        git clone "$REPO_URL" "$REPO_DIR"
    else
        echo "$REPO_NAME já está clonado."
    fi

    # Verificar se já foi construído e instalado
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

# Instalar ferramentas necessárias via Homebrew
echo "Instalando ferramentas via Homebrew..."
brew install cmake extra-cmake-modules qt vulkan-headers ninja vulkan-loader molten-vk pkg-config node glib python@3.12 p7zip

# Instalar dependências
install_dependency "https://github.com/danvratil/qcoro.git" v0.10.0
install_dependency "https://invent.kde.org/frameworks/kirigami.git" v6.2.1

echo "Todas as dependências foram instaladas com sucesso."