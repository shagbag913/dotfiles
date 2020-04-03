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
if empty(glob('~/.config/nvim/autoload/plug.vim'))
  silent !curl -fLo ~/.config/nvim/autoload/plug.vim --create-dirs
    \ https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim
  autocmd VimEnter * PlugInstall --sync | source $MYVIMRC
endif

" Create .vimundo directory
if empty(glob('~/.vimundo'))
    silent !mkdir ~/.vimundo
endif

call plug#begin()
Plug 'scrooloose/nerdcommenter'
call plug#end()

"Persistent undo
set undofile
set undodir=$HOME/.vimundo

" NERD Commenter
let g:NERDSpaceDelims = 1

" Jump to last place
if has("autocmd")
  au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") && @% !~# "COMMIT_EDITMSG" | exe "normal! g'\"" | endif
endif

" Do not fix no EOL
set nofixendofline
