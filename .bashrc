HOSTNAME=$(uname -n)

### COLORS ###
# Text
RED='\e[31m'
CYAN='\e[36m'
GREEN='\e[32m'
REST='\e[39m'
# Background
LIGHTGREY='\e[47m'
DARKGREY='\e[100m'
YELL='\e[103m'
DEF='\e[49m'

# If not running interactively, don't do anything
[[ $- != *i* ]] && return

### PS1 ###
# hostname
PS1="${DARKGREY}${CYAN} \u ${REST}"
# user
PS1+="${LIGHTGREY}${RED} \h ${DEF}${REST}"
# directory
PS1+="${CYAN} \W ${REST}"
# Squigely wiggely
PS1+="${GREEN}$ ${REST}"

if ! neofetch 2> /dev/null; then
  echo "Install neofetch you nub!"
fi

# Source outside files for further configuration
source ${HOME}/.alias
source ${HOME}/.export
source ${HOME}/git-completion.bash
# source common script if on ShagBox
if [[ ${HOSTNAME} = "ShagBox" ]]; then
  if ! source /media/shagbag913/Roms/scripts/common 2> /dev/null; then
    echo "  Please clone scripts from https://github.com/shagbag913/scripts!"
  fi
fi
