HISTFILE=~/.histfile
HISTSIZE=1000
SAVEHIST=1000
unsetopt beep
bindkey -v
zstyle :compinstall filename '/home/shagbag913/.zshrc'

autoload -Uz compinit
compinit

autoload -U colors && colors

[[ -f ~/.fzf.zsh ]] && source ~/.fzf.zsh
[[ -f $HOME/.cache/wal/sequences ]] && cat $HOME/.cache/wal/sequences
. /usr/share/zsh/plugins/zsh-autosuggestions/zsh-autosuggestions.zsh
ZSH_AUTOSUGGEST_USE_ASYNC=true
ZSH_AUTOSUGGEST_STRATEGY=completion
ZSH_AUTOSUGGEST_BUFFER_MAX_SIZE=20
export FZF_DEFAULT_COMMAND='rg --files --hidden'
export PATH="$HOME/.bin:$PATH"
export LC_ALL=C
export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'
export EDITOR=nvim
export USE_CCACHE=1

if [[ $(uname -n) = ShagBox ]]; then
  alias tm='if ! tmux -u attach; then tmux -u; fi'
else
  alias tm='ssh -t $SSHNAME -p $SSHPORT \
            "if ! tmux -u attach; then tmux -u; fi"'
fi
alias .='source'
alias apktool="java -Xmx512M -Dfile.encoding=utf-8 -jar $HOME/.bin/apktool.jar"
alias zrc='source ~/.zshrc'
alias grep='grep --color=auto'
alias l='ls'
alias la='ls -a'
alias ls='ls --color=auto'
alias mirror='sudo reflector --protocol https --latest 50 --number 20 --sort rate --save /etc/pacman.d/mirrorlist'
alias trizen='trizen --skipinteg --noconfirm'
alias wget='wget -c'

reposync() {
    repo sync -j$(($(nproc --all)*2)) --no-clone-bundle --prune --no-tags -c \
        --optimized-fetch -f "$@"
}

rmdl() { rsync -Pvre "ssh -p$SSHPORT" $SSHNAME:"$1" "$2" }
rmul() { rsync -Pvre "ssh -p$SSHPORT" "$1" $SSHNAME:"$2" }

device_zip_list() {
    if [ -n "$1" ]; then
        curl -s https://developers.google.com/android/images | sed -n 's/<td><a href="//p' \
            | sed -n 's/[", ]//gp' | grep $1 | sort
    else
        curl -s https://developers.google.com/android/images | sed -n 's/<td><a href="//p' \
            | sed -n 's/[", ]//gp' | sort
    fi
}

prompt_git_branch() {
    branch="$(git branch |& grep -v fatal | grep '*' | sed 's/[()]//g; s/^* //')"
    [[ -n $branch ]] && echo "($branch) "
}

prompt () {
    PS1="%F{red}%m%k %F{cyan}$(prompt_git_branch)$(basename "$(dirs)") %F{green}$ %f"
}

pre_cmd() {
    prompt
    [[ -f $HOME/.secret ]] && . $HOME/.secret
}

precmd_functions+=(pre_cmd)

[[ -n "$DISPLAY" ]] && neofetch
