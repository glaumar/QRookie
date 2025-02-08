# Public Archive

This repository is a public archive of the QRookie project. The QRookie project is not being actively maintained. If you are interested in maintaining the project, please fork the repository and continue the development.

For now you can use **QRookie Node** project available for Linux, Windows, MacOS and Android Termux at:
https://github.com/victorwads/QRookieNode/releases/

## ![](./icons/64x64/io.github.glaumar.QRookie.png) QRookie 
Download and install Quest games from ROOKIE Public Mirror (like [VRP Rookie Sideloader](https://github.com/VRPirates/rookie) but for Linux and MacOS).

## Screenshots

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

- [QRookie](#-qrookie)
- [Screenshots](#screenshots)
- [Download](#download)
  - [Arch Linux](#arch-linux)
    - [Install from AUR](#install-from-aur)
  - [NixOS](#nixos)
    - [Install directly](#install-directly)
    - [Use with flake.nix (recommended)](#use-with-flakenix-recommended)
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
  - [MacOS](#macos-1)
    - [Install HomeBrew](#install-homebrew)
    - [Build](#build-3)
  - [MacOS (Nix)](#macos-nix)
    - [Install Nix](#install-nix)
    - [Build](#build-4)
    - [Run Without Cloning The Repository](#run-without-cloning-the-repository-1)

---

## Download

### Arch Linux 

![AUR Version](https://img.shields.io/aur/version/qrookie-vrp) ![AUR Last Modified](https://img.shields.io/aur/last-modified/qrookie-vrp)


#### Install from AUR
```shell
paru -S qrookie-vrp
```

### NixOS
#### Install directly
```shell
nix profile install github:glaumar/nur#qrookie
```
#### Use with flake.nix (recommended)
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

### SteamOS And Other Linux
Now there is only a flatpak package, which can run on almost all linux distributions, including SteamOS (Steam Deck).

#### Flathub
![Flathub Version](https://img.shields.io/flathub/v/io.github.glaumar.QRookie) ![Flathub Downloads](https://img.shields.io/flathub/downloads/io.github.glaumar.QRookie)

<a href='https://flathub.org/apps/io.github.glaumar.QRookie'>
    <img width='240' alt='Download on Flathub' src='https://flathub.org/api/badge?locale=en'/>
</a>

#### Flatpak Bundle
![GitHub Release](https://img.shields.io/github/v/release/glaumar/QRookie) ![GitHub Release Date](https://img.shields.io/github/release-date/Glaumar/QRookie) ![GitHub Downloads (specific asset, all releases)](https://img.shields.io/github/downloads/Glaumar/QRookie/QRookie.flatpak)  


See [releases](https://github.com/glaumar/QRookie/releases).

### MacOS

#### MacOS Bundle
![GitHub Downloads (specific asset, all releases)](https://img.shields.io/github/downloads/Glaumar/QRookie/QRookie_MacOs_arm64.dmg) ![GitHub Downloads (specific asset, all releases)](https://img.shields.io/github/downloads/Glaumar/QRookie/QRookie_MacOs_x86_64.dmg)  

See [releases](https://github.com/glaumar/QRookie/releases).

> [!WARNING]
> *The MacOS Bundle is automatically built using a github action and is not signed or notarized*

> [!WARNING]
> *The MacOS Bundle does not include **java runtime**. If you want to use the `rename pkg` function, you need to install the java runtime additionally:*
> ```shell
> brew install java
> 
> sudo ln -sfn /usr/local/opt/openjdk/libexec/openjdk.jdk /Library/Java/JavaVirtualMachines/openjdk.jdk
> ```
>

#### Install using nix  

> [!WARNING]
> *If you don’t know nix at all, we don’t recommend using this method to install QRookie.*

> [!WARNING]
> *More than 3GB of data may be downloaded during the first installation*

1. **Install nix:** We recommend using the [graphical installer](https://determinate.systems/posts/graphical-nix-installer/) for installation. （From [here](https://nixcademy.com/2024/01/15/nix-on-macos/) you can find more ways to install nix on MacOS）

2. **Install QRookie using flake:** 
```shell
export NIXPKGS_ALLOW_UNSUPPORTED_SYSTEM=1

nix profile install --impure github:glaumar/nur#qrookie \
--extra-experimental-features nix-command \
--extra-experimental-features flakes

ln -s ~/.nix-profile/Applications/QRookie.app /Applications
```

#### nix-darwin
Refer to [NixOS configuration](#use-with-flakenix-recommended)

## FAQ

### 1. Game installation failed?
QRookie currently does not support `install.txt`. Some games may fail to install or may not run after installation. You can find the downloaded games in:
- `~/.local/share/QRookie/` (ArchLinux、 NixOS)
- `~/.var/app/io.github.glaumar.QRookie/data/QRookie/` （Flatpak）
- `~/Library/Application Support/QRookie` (MacOS)  

and manually install them ([Howto: Sideload Manually](https://vrpirates.wiki/en/Howto/Manual-Sideloading)).

### 2. Are all the games provided by QRookie? Are these games infected with viruses?

No, QRookie does not provide any games. It is just a downloader that downloads games from public servers. QRookie does not know what it is downloading, and it does not check whether the downloaded content is a legitimate game or a virus. **To ensure the safety of your VR device, please do not use QRookie** .

## Develop
### Archlinux
#### Install Dependencies
```shell
sudo pacman -S gcc cmake qt6-base qt6-declarative qcoro-qt6 kirigami extra-cmake-modules qqc2-breeze-style xdg-utils p7zip android-tools 
```
Some packages need to be downloaded from AUR
```shell
paru -S android-sdk-build-tools android-apktool
```

#### Build
```shell
# build
cmake -B build -S . -DCMAKE_BUILD_TYPE='Release' -DCMAKE_INSTALL_PREFIX='/usr'
cmake --build build --target QRookie

# run
./build/QRookie

# install (not recommended)
cmake --install build
```

### Flatpak
#### Install Runtime
```shell
flatpak install \
    runtime/org.kde.Platform/x86_64/6.6\
    runtime/org.kde.Sdk/x86_64/6.6\
    runtime/org.freedesktop.Sdk.Extension.openjdk21/x86_64/23.08
```

#### Build
##### Build Only
```shell
flatpak-builder build_flatpak  io.github.glaumar.QRookie.yml --force-clean
```

##### Build and Install
```shell
flatpak-builder build_flatpak  io.github.glaumar.QRookie.yml --force-clean --install --user
```

##### Export Flatpak Bundle
```shell
flatpak build-export export build_flatpak && flatpak build-bundle export QRookie.flatpak io.github.glaumar.QRookie --runtime-repo=https://flathub.org/repo/flathub.flatpakrepo
```

### NixOS
#### Enable Flakes 
Add 
```nix
nix.settings.experimental-features = [ "nix-command" "flakes" ];
```
to your `/etc/nixos/configuration.nix` , then run 
```
sudo nixos-rebuild switch
``` 
for more details, please refer to [here](https://nixos-and-flakes.thiscute.world/nixos-with-flakes/nixos-with-flakes-enabled)

#### Build
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

#### Run Without Cloning The Repository
```shell
nix run github:glaumar/QRookie
```

### MacOS
#### Install HomeBrew
```shell
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### Build
```shell
cd ./macOs/

# install dependencies, build and run
./buildMacOs.sh  --open
```

### MacOS (Nix)
#### Install Nix
We recommend using the [graphical installer](https://determinate.systems/posts/graphical-nix-installer/) for installation.  

For more details, see [here](https://nixcademy.com/2024/01/15/nix-on-macos/)

#### Build
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

#### Run Without Cloning The Repository
```shell
export NIXPKGS_ALLOW_UNSUPPORTED_SYSTEM=1

nix run --impure github:glaumar/QRookie
```
    
    
---
![](https://badges.pufler.dev/visits/glaumar/QRookie)
