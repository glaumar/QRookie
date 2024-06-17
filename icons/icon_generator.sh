#!/usr/bin/env bash

# Define os tamanhos dos ícones

# Função para criar ícones no Linux usando Inkscape
generate_icons_linux() {
  sizes=(16 32 48 64 128 256)
  for size in "${sizes[@]}"; do
    inkscape scalable/io.github.glaumar.QRookie.svg -w "$size" -h "$size" -o "${size}x${size}/io.github.glaumar.QRookie.png"
  done
}

# Função para criar ícones no macOS usando sips e iconutil
generate_icons_macos() {
  # Diretório temporário para armazenar os ícones
  iconset_dir="icon.iconset"
  png_filename="256x256/io.github.glaumar.QRookie.png"
  mkdir -p "$iconset_dir"

  # Copiar a imagem de 256x256 para o diretório iconset
  cp "$png_filename" "$iconset_dir/icon_256x256.png"

  for size in "${sizes[@]}"; do
    if [ "$size" -ne 256 ]; then
      sips -z "$size" "$size" "$png_filename" --out "${iconset_dir}/icon_${size}x${size}.png"
    fi
    # Gerar ícones de 2x (retina) também
    retina_size=$((size * 2))
    sips -z "$retina_size" "$retina_size" "$png_filename" --out "${iconset_dir}/icon_${size}x${size}@2x.png"
  done

  # Cria o arquivo .icns a partir do diretório iconset
  iconutil -c icns -o "icon.icns" "$iconset_dir"

  # Limpa o diretório temporário
  rm -r "$iconset_dir"
}

# Detecta o sistema operacional e executa a função apropriada
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  generate_icons_linux
elif [[ "$OSTYPE" == "darwin"* ]]; then
  generate_icons_macos
else
  echo "Sistema operacional não suportado."
  exit 1
fi