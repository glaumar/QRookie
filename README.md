# ![](./icons/64x64/io.github.glaumar.QRookie.png) QRookie 
Download and install Quest games from ROOKIE Public Mirror (like [VRP Rookie Sideloader](https://github.com/VRPirates/rookie) but for linux).

# Screenshots

<div align="center">
  <img src="screenshots/Screenshot1.png" width="640"/>
</div>
<details>
<summary>Click for more screenshots</summary>
    <div align="center">
        <img src="screenshots/Screenshot2.png" width="640"/>
        <img src="screenshots/Screenshot3.png" width="640"/>
    </div>
</details>

---

- [ QRookie](#-qrookie)
- [Screenshots](#screenshots)
- [Download](#download)
  - [Arch Linux](#arch-linux)
    - [Install from AUR](#install-from-aur)
  - [NixOS](#nixos)
    - [Install directly](#install-directly)
    - [Use with flake (recommended)](#use-with-flake-recommended)
  - [SteamOS And Other Linux](#steamos-and-other-linux)
    - [Flathub](#flathub)
    - [Flatpak Bundle](#flatpak-bundle)
  - [MacOS](#macos)
    - [MacOS Bundle](#macos-bundle)
    - [Install using nix](#install-using-nix)
    - [nix-darwin](#nix-darwin)
- [FAQ](#faq)
  - [1. Game installation failed?](#1-game-installation-failed)
  - [2. Are all the games provided by QRookie? Are these games infected with viruses?](#2-are-all-the-games-provided-by-qrookie-are-these-games-infected-with-viruses)
- [Develop](#develop)
  - [Archlinux](#archlinux)
    - [Install Dependencies](#install-dependencies)
    - [Build](#build)
  - [Flatpak](#flatpak)
    - [Install Runtime](#install-runtime)
    - [Build](#build-1)
      - [Build Only](#build-only)
      - [Build and Install](#build-and-install)
      - [Export Flatpak Bundle](#export-flatpak-bundle)
  - [NixOS](#nixos-1)
    - [Enable Flakes](#enable-flakes)
    - [Build](#build-2)
    - [Run Without Cloning The Repository](#run-without-cloning-the-repository)
  - [MacOS (Using Nix)](#macos-using-nix)
    - [Install Nix](#install-nix)
    - [Build](#build-3)
    - [Run Without Cloning The Repository](#run-without-cloning-the-repository-1)

---

# Download

## Arch Linux 

![AUR Version](https://img.shields.io/aur/version/qrookie-vrp) ![AUR Last Modified](https://img.shields.io/aur/last-modified/qrookie-vrp)


### Install from AUR
```shell
paru -S qrookie-vrp
```

## NixOS
### Install directly
```shell
nix profile install github:glaumar/nur#qrookie
```
### Use with flake (recommended)
Modify your flake.nix like：

```nix
# flake.nix
{
  inputs.glaumar_repo = {
    url = "github:glaumar/nur";
    inputs.nixpkgs.follows = "nixpkgs";
  };

  # ...

  outputs = {nixpkgs, ...} @ inputs: {
    nixosConfigurations.HOSTNAME = nixpkgs.lib.nixosSystem {
      modules = [
        ({
          nixpkgs.overlays = [
            (final: prev: {
              glaumar_repo = inputs.glaumar_repo.packages."${prev.system}";
            })
          ];
        })

        # ...
      ];
    };
  }
}
```
Then you can use `pkgs.glaumar_repo.qrookie` in your other configuration modules.

## SteamOS And Other Linux
Now there is only a flatpak package, which can run on almost all linux distributions, including SteamOS (Steam Deck).

### Flathub
![Flathub Version](https://img.shields.io/flathub/v/io.github.glaumar.QRookie) ![Flathub Downloads](https://img.shields.io/flathub/downloads/io.github.glaumar.QRookie)

<a href='https://flathub.org/apps/io.github.glaumar.QRookie'>
    <img width='240' alt='Download on Flathub' src='https://flathub.org/api/badge?locale=en'/>
</a>

### Flatpak Bundle
![GitHub Release](https://img.shields.io/github/v/release/glaumar/QRookie) ![GitHub Release Date](https://img.shields.io/github/release-date/Glaumar/QRookie) ![GitHub Downloads (specific asset, all releases)](https://img.shields.io/github/downloads/Glaumar/QRookie/QRookie.flatpak)  


See [releases](https://github.com/glaumar/QRookie/releases).

## MacOS

### MacOS Bundle
![GitHub Downloads (specific asset, all releases)](https://img.shields.io/github/downloads/Glaumar/QRookie/QRookie_MacOs_arm64.dmg) ![GitHub Downloads (specific asset, all releases)](https://img.shields.io/github/downloads/Glaumar/QRookie/QRookie_MacOs_x86_64.dmg)  

See [releases](https://github.com/glaumar/QRookie/releases).

### Install using nix  

**⚠️ If you don’t know nix at all, we don’t recommend using this method to install it.**  

We provide an installation script
```shell
sh <(curl -L https://raw.githubusercontent.com/glaumar/QRookie/main/macOs/installMacOSNix.sh)
```

Or you can try to install them manually:

**install nix:**  

I recommend using the [graphical installer](https://determinate.systems/posts/graphical-nix-installer/) for installation. （From [here](https://nixcademy.com/2024/01/15/nix-on-macos/) you can find more ways to install nix on MacOS）

**install QRookie using flake：** 

```shell
export NIXPKGS_ALLOW_UNSUPPORTED_SYSTEM=1

nix profile install --impure github:glaumar:nur#qrookie \
--extra-experimental-features nix-command \
--extra-experimental-features flakes

ln -s ~/.nix-profile/Applications/QRookie.app /Applications
```



### nix-darwin
- [ ] TODO



# FAQ

## 1. Game installation failed?
QRookie currently does not support `install.txt`. Some games may fail to install or may not run after installation. You can find the downloaded games in `~/.var/app/io.github.glaumar.QRookie/data/QRookie/` or `~/.local/share/QRookie/` and manually install them ([ Howto: Sideload Manually](https://vrpirates.wiki/en/Howto/Manual-Sideloading)).

## 2. Are all the games provided by QRookie? Are these games infected with viruses?

No, QRookie does not provide any games. It is just a downloader that downloads games from public servers. QRookie does not know what it is downloading, and it does not check whether the downloaded content is a legitimate game or a virus. **To ensure the safety of your VR device, please do not use QRookie** .

# Develop
## Archlinux
### Install Dependencies
```shell
sudo pacman -S gcc cmake qt6-base qt6-declarative qcoro-qt6 kirigami extra-cmake-modules qqc2-breeze-style xdg-utils p7zip android-tools 
```
Some packages need to be downloaded from AUR
```shell
paru -S android-sdk-build-tools android-apktool
```

### Build
```shell
# build
cmake -B build -S . -DCMAKE_BUILD_TYPE='Release' -DCMAKE_INSTALL_PREFIX='/usr'
cmake --build build --target qrookie

# run
./build/qrookie

# install (not recommended)
cmake --install build
```

## Flatpak
### Install Runtime
```shell
flatpak install \
    runtime/org.kde.Platform/x86_64/6.6\
    runtime/org.kde.Sdk/x86_64/6.6\
    runtime/org.freedesktop.Sdk.Extension.openjdk21/x86_64/23.08
```

### Build
#### Build Only
```shell
flatpak-builder build_flatpak  io.github.glaumar.QRookie.yml --force-clean
```

#### Build and Install
```shell
flatpak-builder build_flatpak  io.github.glaumar.QRookie.yml --force-clean --install --user
```

#### Export Flatpak Bundle
```shell
flatpak build-export export build_flatpak && flatpak build-bundle export QRookie.flatpak io.github.glaumar.QRookie --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
```

## NixOS
### Enable Flakes 
Add 
```nix
nix.settings.experimental-features = [ "nix-command" "flakes" ];
```
to your `/etc/nixos/configuration.nix` , then run 
```
sudo nixos-rebuild switch
``` 
for more details, please refer to [here](https://nixos-and-flakes.thiscute.world/nixos-with-flakes/nixos-with-flakes-enabled)

### Build
```shell
nix build

# run
./result/bin/qrookie
```
or
```shell
# Create an shell that includes development tools and all dependencies
nix develop

# Then use CMake to build
cmake -B build -S .
cmake --build build --target qrookie

# run
./build/qrookie
```

### Run Without Cloning The Repository
```shell
nix run github:glaumar/QRookie
```

## MacOS (Using Nix)
### Install Nix
I recommend using the [graphical installer](https://determinate.systems/posts/graphical-nix-installer/) for installation.  
For more details, refer to [here](https://nixcademy.com/2024/01/15/nix-on-macos/)

### Build
Some dependencies in Nix are not supported on Darwin, but they can actually run if you force them to compile.  
```shell
export NIXPKGS_ALLOW_UNSUPPORTED_SYSTEM=1
```
```shell
nix build --impure

# run
./result/bin/qrookie
```
or
```shell
# Create an shell that includes development tools and all dependencies
nix develop --impure

# Then use CMake to build
cmake -B build -S .
cmake --build build --target qrookie

# run
./build/qrookie
```

### Run Without Cloning The Repository
```shell
export NIXPKGS_ALLOW_UNSUPPORTED_SYSTEM=1

nix run --impure github:glaumar/QRookie
```
    
    
---
![](https://badges.pufler.dev/visits/glaumar/QRookie)
