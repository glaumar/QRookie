export PATH=$HOME/bin:$HOME/.cargo/bin:$PATH
export BROWSER=firefox
export EDITOR=vim

#if [ $XDG_SESSION_TYPE = "wayland" ];then
#    # fcitx
#    # /etc/profile.d/input-support.sh 中默认设置了一些输入法相关的环境变量，根据 https://fcitx-im.org/wiki/Using_Fcitx_5_on_Wayland#KDE_Plasma，wayland下fcitx不要设置 GTK_IM_MODULE,QT_IM_MODULE,SDL_IM_MODULE
#    export XMODIFIERS=@im=fcitx
#    unset GTK_IM_MODULE
#    unset QT_IM_MODULE
#    unset SDL_IM_MODULE
#    
#    export CLUTTER_BACKEND=wayland
#    export SDL_VIDEODRIVER=wayland
#    export QT_QPA_PLATFORM=wayland
#
#    # firefox
#    export MOZ_ENABLE_WAYLAND=1
#
#else
#    # fcitx
#    im=fcitx
#    export GTK_IM_MODULE=$im
#    export QT_IM_MODULE=$im
#    export XMODIFIERS=@im=$im
#    export INPUT_METHOD=$im
#    export SDL_IM_MODULE=$im
#fi

