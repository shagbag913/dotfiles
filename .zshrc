HISTFILE=~/.histfile
HISTSIZE=1000
SAVEHIST=1000
setopt share_history
unsetopt beep
bindkey -v
zstyle :compinstall filename '/home/shagbag913/.zshrc'

autoload -Uz compinit
autoload -U colors && colors
compinit

. $HOME/.zsh/zsh-autosuggestions/zsh-autosuggestions.zsh
ZSH_AUTOSUGGEST_USE_ASYNC=true
ZSH_AUTOSUGGEST_STRATEGY=completion
ZSH_AUTOSUGGEST_BUFFER_MAX_SIZE=20

. $HOME/.zsh/zsh-history-substring-search/zsh-history-substring-search.zsh
bindkey '^[[A' history-substring-search-up
bindkey '^[[B' history-substring-search-down

export PATH="$HOME/.bin:$HOME/Android/Sdk/build-tools/29.0.1:$PATH"
[[ -d $HOME/.depot_tools ]] && export PATH="$PATH:$HOME/.depot_tools"
export LC_ALL=C
export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'
export EDITOR=nvim
export KEYTIMEOUT=1

# Ccache
export USE_CCACHE=1
export CCACHE_EXEC=$(which ccache)
export CCACHE_MAXSIZE=100G
export CCACHE_SLOPPINESS=locale

# ALIAS'S
if [[ $(uname -n) = ShagBox ]]; then
  alias tm='if ! tmux -u attach; then tmux -u; fi'
else
  alias tm='ssh -t $SSHNAME -p $SSHPORT \
            "if ! tmux -u attach; then tmux -u; fi"'
fi
alias .='source'
alias apktool="java -Xmx512M -Dfile.encoding=utf-8 -jar $HOME/.bin/apktool.jar"
alias e='nvim'
alias zrc='source ~/.zshrc'
alias grep='grep --color=auto'
alias l='ls'
alias la='ls -a'
alias ls='ls --color=auto'
alias mirror='sudo reflector --protocol https --latest 50 --number 20 --sort rate --save /etc/pacman.d/mirrorlist'
alias rs='repo sync --no-clone-bundle --prune --no-tags --no-clone-bundle -c --optimized-fetch -j8'
alias s!='sudo $(fc -ln -1)'
alias wget='wget -c'
alias ga='git add'
alias gaa='git add --all'
alias gap='git add --all --patch'
alias gc='git commit --gpg-sign'
alias gca='git commit --gpg-sign --amend'
alias gcad='git commit --gpg-sign --amend --date=now'
alias gcp='git cherry-pick --gpg-sign'
alias gcpa='git cherry-pick --abort'
alias gcpc='git cherry-pick --continue'
alias gf='git fetch'
alias gm='git merge'
alias gma='git merge --abort'
alias gma='git merge --continue'
alias gp='git push'
alias gpl='git pull'
alias gs='git status'
alias gr='git revert'
alias gl='git log'
alias glo='git log --oneline'
alias glp='git log -p'
alias rg="rg -j$(($(nproc --all)*2))"

ghp() {
    [[ -z "$1" || -z "$2" ]] && { echo "Missing args (./$0 <repo> <branch>)"; return 1; }
    git push https://github.com/$1 HEAD:refs/heads/$2
}
reposync() {
    repo sync -j6 -c --no-clone-bundle "$@"
}
b() { for ((i=0;i<$1;i++)); do cd ..; done }
rmdl() { rsync -Pvre "ssh -p$SSHPORT" $SSHNAME:"$1" "$2" }
rmul() { rsync -Pvre "ssh -p$SSHPORT" "$1" $SSHNAME:"$2" }

prompt_git_branch() {
    branch=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || return)
    echo "\e[3m$branch\e[0m "
}

set_prompts() {
    OLDRET=$?
    if [[ "$OLDRET" != 0 ]]; then
        RPROMPT="%F{red}$OLDRET :("
    else
        RPROMPT=""
    fi
    PROMPT=" %F{red}%n%F{cyan}@%F{red}%m %F{cyan}$(prompt_git_branch)%F{cyan}$(dirs)"$'\n'" %F{green}$%f "
}

pre_cmd() {
    set_prompts
    [[ -f $HOME/.secret ]] && . $HOME/.secret
}

precmd_functions+=(pre_cmd)

neofetch
