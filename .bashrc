# If not running interactively, don't do anything
[[ $- != *i* ]] && return

# Terminal color theme
cat ${HOME}/.colors

### COLORS ###
RED='\e[31m'
CYAN='\e[36m'
GREEN='\e[32m'
YELLOW='\e[33m'
REST='\e[39m'

### PS1 ###
# hostname
PS1="\[${CYAN}\]\u@\[${RED}\]\h\[${CYAN}\] \W \[${GREEN}\]$ \[${REST}\]"

# Source outside files for further configuration
for file in 'alias' 'export' git-completion.bash secret; do
    . $HOME/.$file
done

# Show neofetch when opening terminal to look cool
neofetch
