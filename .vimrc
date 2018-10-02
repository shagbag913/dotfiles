syntax enable
set tabstop=2
set softtabstop=0
set expandtab
set shiftwidth=2
set smarttab
set autoindent
set smartindent
set incsearch
set hlsearch
set lazyredraw

execute pathogen#infect()

call plug#begin()
Plug 'dylanaraps/wal.vim'
call plug#end()

colorscheme wal

