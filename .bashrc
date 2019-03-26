cat ${HOME}/.colors

### COLORS ###
# Text
RED='\e[31m'
CYAN='\e[36m'
GREEN='\e[32m'
YELLOW='\e[33m'
REST='\e[39m'

# If not running interactively, don't do anything
[[ $- != *i* ]] && return

### PS1 ###
# hostname
PS1="\[${CYAN}\]\u@\[${RED}\]\h\[${CYAN}\] \W \[${GREEN}\]$ \[${REST}\]"

# Source outside files for further configuration
source ${HOME}/.alias
source ${HOME}/.export
source ${HOME}/.git-completion.bash
source $HOME/.secret

neofetch
