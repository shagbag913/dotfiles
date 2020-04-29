filetype plugin on

" Indention
set autoindent
set smartindent

set encoding=utf-8

" Search
set incsearch
set hlsearch

" Install vim-plug
if empty(glob('~/.vim/autoload/plug.vim'))
  silent !curl -fLo ~/.vim/autoload/plug.vim --create-dirs
    \ https://raw.githubusercontent.com/junegunn/vim-plug/master/plug.vim
  autocmd VimEnter * PlugInstall --sync | source $MYVIMRC
endif

" Jump to last place
if has("autocmd")
  au BufReadPost * if line("'\"") > 1 && line("'\"") <= line("$") && @% !~# "COMMIT_EDITMSG" | exe "normal! g'\"" | endif
endif

" Do not fix no EOL
set nofixendofline

" Create .vimundo directory
if empty(glob('~/.vimundo'))
    silent !mkdir ~/.vimundo
endif

call plug#begin()
Plug 'scrooloose/nerdcommenter'
Plug 'ycm-core/YouCompleteMe'
Plug 'joshdick/onedark.vim'
Plug 'sheerun/vim-polyglot'
Plug 'Yggdroot/indentLine'
Plug 'tpope/vim-sleuth'
Plug 'ludovicchabant/vim-gutentags'
Plug 'majutsushi/tagbar'
call plug#end()

" Persistent undo
set undofile
set undodir=$HOME/.vimundo

" Never fix EOF, for the sake of keeping diffs clean
set nofixendofline

" Enable 256 color support
set t_Co=256
set termguicolors

let &t_8f = "\<Esc>[38;2;%lu;%lu;%lum"
let &t_8b = "\<Esc>[48;2;%lu;%lu;%lum"

let g:onedark_terminal_italics = 1

colorscheme onedark

" Clear background
hi! Normal ctermbg=NONE guibg=NONE
hi! NonText ctermbg=NONE guibg=NONE

" YouCompleteMe
let g:ycm_autoclose_preview_window_after_completion = 1
let g:ycm_autoclose_preview_window_after_insertion = 1
let g:ycm_collect_identifiers_from_tags_files = 1
let g:ycm_seed_identifiers_with_syntax = 1

" Gutentags
let g:gutentags_ctags_extra_args = ['--fields=+l']

" indentLine
let g:indentLine_showFirstIndentLevel = 1
let g:indentLine_char = '┊'
let g:indentLine_first_char = '┊'

" NERDCommenter
let g:NERDSpaceDelims = 1
let g:NERDCommentEmptyLines = 1
let g:NERDTrimTrailingWhitespace = 1

let mapleader = ","

nnoremap <Leader>t :TagbarToggle<CR>
