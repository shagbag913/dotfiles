#
# ~/.bash_profile
#

[[ -f ~/.bashrc ]] && . ~/.bashrc

[[ ! $DISPLAY && $XDG_VTNR = 1 ]] && exec startx
