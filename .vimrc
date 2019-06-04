syntax enable

filetype plugin indent on

set softtabstop=4
set shiftwidth=4
set expandtab

set autoindent
set smartindent

set incsearch
set hlsearch
set lazyredraw

set pastetoggle=<F5>

execute pathogen#infect()

call plug#begin()
Plug 'NLKNguyen/papercolor-theme'
call plug#end()

set background=dark
colorscheme PaperColor
