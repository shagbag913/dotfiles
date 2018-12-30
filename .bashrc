cat ${HOME}/.colors

HOSTNAME=$(uname -n)

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
PS1="\[${CYAN}\]\u@\[${REST}\]"
# user
PS1+="\[${RED}\]\h\[${REST}\]"
# directory
PS1+="\[${CYAN}\] \W \[${REST}\]"
# Dollah sign bcuz why not
PS1+="\[${GREEN}\]$ \[${REST}\]"

# Source outside files for further configuration
source ${HOME}/.alias
source ${HOME}/.export
source ${HOME}/.git-completion.bash
[[ -f $HOME/.secret ]] && source $HOME/.secret

neofetch

export LC_ALL=C
PATH=~/.bin:$PATH
