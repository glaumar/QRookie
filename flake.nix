{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, utils, ... }:
    utils.lib.eachDefaultSystem
      (system:
        let
          pkgs = import nixpkgs {
            inherit system;
            config.android_sdk.accept_license = true;
            config.allowUnfree = true;
          };
          buildToolsVersion = "34.0.0";
          androidComposition = pkgs.androidenv.composeAndroidPackages {
            buildToolsVersions = [ buildToolsVersion ];
          };
          zipAlignPath = "${androidComposition.androidsdk}/libexec/android-sdk/build-tools/${buildToolsVersion}";
          boldPanelIcons = false;
          blackPanelIcons = false;
          alternativeIcons = false;
          themeVariants = [];
        in
        {
          packages.default = pkgs.stdenv.mkDerivation {
            pname = "qrookie";
            version = "0.3.4";

            src = ./.;

            nativeBuildInputs = with pkgs; [
              cmake
              qt6.wrapQtAppsHook
              kdePackages.extra-cmake-modules
            ];

            buildInputs = with pkgs; [
              kdePackages.qtbase
              kdePackages.qtdeclarative
              kdePackages.qcoro
              kdePackages.kirigami
              kdePackages.qtsvg
              kdePackages.qtimageformats
            ] ++ lib.optionals stdenv.isLinux [ 
              kdePackages.qqc2-breeze-style 
            ] ++ lib.optionals stdenv.isDarwin [
              kdePackages.breeze-icons
            ];

            qtWrapperArgs = with pkgs;[
              ''
                --prefix PATH : ${lib.makeBinPath [ p7zip apktool xdg-utils android-tools apksigner jdk21_headless ]}  
                --prefix PATH : ${zipAlignPath}
              ''
            ];

            cmakeFlags = [ "-DCMAKE_BUILD_TYPE=Release" ];

            meta = with pkgs.lib; {
              homepage = "https://github.com/glaumar/QRookie";
              description = ''
                Download and install Quest games from ROOKIE Public Mirror.
              '';
              licencse = licenses.gpl3;
              platforms = platforms.all;
            };
          };

          devShells.default = pkgs.mkShell rec {
            buildInputs = with pkgs; [
              stdenv.cc.cc
              clang-tools
              cmake
              kdePackages.extra-cmake-modules
              kdePackages.qtbase
              kdePackages.qtdeclarative
              kdePackages.qcoro
              kdePackages.kirigami
              kdePackages.qtsvg
              kdePackages.qtimageformats
              p7zip
              apktool
              xdg-utils
              android-tools
              apksigner
              jdk21_headless
            ] ++ lib.optionals stdenv.isLinux [ 
              gdb kdePackages.qqc2-breeze-style 
            ] ++ lib.optionals stdenv.isDarwin [
              kdePackages.breeze-icons
            ];

            shellHook = ''
              PATH="${zipAlignPath}:$PATH"
            '';

            LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath buildInputs;
          };
        }
      )
    // {
      overlays.default = self: pkgs: {
        qrookie = self.packages."${pkgs.system}".qrookie;
      };
    };
}
