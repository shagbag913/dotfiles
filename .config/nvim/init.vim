syntax enable
filetype plugin indent on

"Tab key behavior
set softtabstop=4
set shiftwidth=4
set expandtab

"Indention
set autoindent
set smartindent

"Search
set incsearch
set hlsearch

"Mappings
set pastetoggle=<C-x>
nnoremap <silent> <C-w> :noh<CR>

"Plug
call plug#begin()
Plug 'farmergreg/vim-lastplace'
call plug#end()
