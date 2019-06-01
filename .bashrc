# Return if not running interactively
[[ $- != *i* ]] && return

# Terminal color theme
cat $HOME/.colors

# Source private variables
[[ -f $HOME/.secret ]] && . $HOME/.secret

# Source git completion
. $HOME/.git-completion.bash

# Amount of jobs for make
MAKEFLAGS=$(echo "$(nproc --all) * 1.75" | bc -l | sed 's/\..*//')

#############
# VARIABLES #
#############
if [[ $HOSTNAME = "ShagBox" ]]; then
    export USE_CCACHE=1
    PATH="$HOME/Android/Sdk/build-tools/28.0.3:$PATH"
fi
PS1='\[\e[36m\]\u@\[\e[31m\]\h\[\e[36m\] \W \[\e[32m\]$ \[\e[39m\]'
PS2='\e[31m>\e[39m '
PATH="$HOME/.bin:$PATH"
export LC_ALL=C
export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'
export EDITOR=vim

###########
# ALIAS'S #
###########
if [[ $(uname -n) = ShagBox ]]; then
  alias tm='if ! tmux -u attach; then tmux -u; fi'
else
  alias tm='ssh -t $SSHNAME -p $SSHPORT \
            "if ! tmux -u attach; then tmux -u; fi"'
fi
alias apktool="java -Xmx512M -Dfile.encoding=utf-8 -jar $HOME/.bin/apktool.jar"
alias brc='source ~/.bashrc'
alias grep='grep --color=auto'
alias pgrep='find . -type f -print0  | xargs -0 -P 4 grep -n'
alias l='ls'
alias la='ls -a'
alias ls='ls --color=auto'
alias mirror='sudo reflector --protocol https --latest 50 --number 20 --sort rate --save /etc/pacman.d/mirrorlist'
alias trizen='trizen --skipinteg --noconfirm'
alias wget='wget -c'

#############
# FUNCTIONS #
#############
reposync() {
    repo sync -j$MAKEFLAGS --no-clone-bundle --prune --no-tags -c \
        --optimized-fetch -f "$@"
}

rmdl() {
    if [[ $HOSTNAME != "ShagBook" ]]; then
        echo "$(basename $0) is meant to be run on ShagBook!"
        exit 1
    fi

    rsync -Pvre "ssh -p$SSHPORT" $SSHNAME:"$1" "$2"
}

rmul() {
    if [[ $HOSTNAME != "ShagBook" ]]; then
        echo "$(basename $0) is meant to be run on ShagBook!"
        exit 1
    fi

    rsync -Pvre "ssh -p$SSHPORT" "$1" $SSHNAME:"$2"
}

# Show neofetch when opening terminal to look cool
neofetch
