# If not running interactively, don't do anything
[[ $- != *i* ]] && return

# Terminal color theme
cat ${HOME}/.colors

PS1="\[\e[36m\]\u@\[\e[31m\]\h\[\e[36m\] \W \[\e[32m\]$ \[\e[39m\]"

# Source outside files for further configuration
for file in 'alias' 'export' git-completion.bash secret; do
    . $HOME/.$file
done

# Show neofetch when opening terminal to look cool
neofetch
