syntax enable
filetype plugin indent on
set encoding=utf-8

" Tab key behavior
set softtabstop=4
set shiftwidth=4
set expandtab

" Indention
set autoindent
set smartindent

" Search
set incsearch
set hlsearch

" Mappings
set pastetoggle=<C-x>
nnoremap <silent> <C-w> :noh<CR>

" Plug
if empty(glob('~/.vim/autoload/plug.vim'))
    silent !curl -fLo ~/.vim/autoload/plug.vim --create-dirs
        \ https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim
    autocmd VimEnter * PlugInstall --sync | source $MYVIMRC
endif

" Create .vimundo directory
if empty(glob('~/.vimundo'))
    silent !mkdir ~/.vimundo
endif

call plug#begin()
Plug 'neoclide/coc.nvim', {'branch': 'release'}
Plug 'farmergreg/vim-lastplace'
Plug 'scrooloose/nerdtree'
Plug 'scrooloose/nerdcommenter'
call plug#end()

" Persistent undo
set undofile
set undodir=$HOME/.vimundo

"  Use tab for trigger completion with characters ahead and navigate.
"  Use command ':verbose imap <tab>' to make sure tab is not mapped by other plugin.
inoremap <silent><expr> <TAB>
      \ pumvisible() ? "\<C-n>" :
      \ <SID>check_back_space() ? "\<TAB>" :
      \ coc#refresh()
inoremap <expr><S-TAB> pumvisible() ? "\<C-p>" : "\<C-h>"

function! s:check_back_space() abort
  let col = col('.') - 1
  return !col || getline('.')[col - 1]  =~# '\s'
endfunction

" Toggle NERDTree
nmap <C-N> :NERDTreeToggle<CR>

" NERD Commenter
let g:NERDSpaceDelims = 1
