HISTFILE=~/.histfile
HISTSIZE=1000
SAVEHIST=1000
unsetopt beep
bindkey -v
zstyle :compinstall filename '/home/shagbag913/.zshrc'

autoload -Uz compinit
compinit

autoload -U colors && colors

cat $HOME/.colors
[[ -f $HOME/.secret ]] && . $HOME/.secret
PATH="$HOME/.bin:$PATH"
export LC_ALL=C
export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'
export EDITOR=vim

if [[ $(uname -n) = ShagBox ]]; then
  alias tm='if ! tmux -u attach; then tmux -u; fi'
else
  alias tm='ssh -t $SSHNAME -p $SSHPORT \
            "if ! tmux -u attach; then tmux -u; fi"'
fi
alias apktool="java -Xmx512M -Dfile.encoding=utf-8 -jar $HOME/.bin/apktool.jar"
alias brc='source ~/.bashrc'
alias grep='grep --color=auto'
alias l='ls'
alias la='ls -a'
alias ls='ls --color=auto'
alias mirror='sudo reflector --protocol https --latest 50 --number 20 --sort rate --save /etc/pacman.d/mirrorlist'
alias trizen='trizen --skipinteg --noconfirm'
alias wget='wget -c'

reposync() {
    repo sync -j$MAKEFLAGS --no-clone-bundle --prune --no-tags -c \
        --optimized-fetch -f "$@"
}

rmdl() { rsync -Pvre "ssh -p$SSHPORT" $SSHNAME:"$1" "$2" }

rmul() { rsync -Pvre "ssh -p$SSHPORT" "$1" $SSHNAME:"$2" }

ps -e | grep Xorg &>/dev/null && neofetch
