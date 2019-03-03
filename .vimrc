syntax enable

filetype plugin indent on

set tabstop=4
set softtabstop=4
set shiftwidth=4
set expandtab

set autoindent
set smartindent

set incsearch
set hlsearch
set lazyredraw

set pastetoggle=<F5>

set foldmethod=indent
set nofoldenable
set foldnestmax=10
nnoremap <Space> zA

execute pathogen#infect()

call plug#begin()
call plug#end()
