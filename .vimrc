syntax enable
filetype plugin indent on
set termguicolors

set encoding=utf-8
scriptencoding utf-8

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
Plug 'nathanaelkane/vim-indent-guides'
Plug 'Yggdroot/indentLine'
call plug#end()

"Persistent undo
set undofile
set undodir=$HOME/.vimundo

"Indention
let g:indentLine_char = '┋'
let g:indentLine_showFirstIndentLevel = 1
let g:indentLine_first_char = '┋'
let g:indentLine_color_gui = '#808080'
set list lcs=tab:\┋\ 
hi SpecialKey guifg=#808080

" Use tab for trigger completion with characters ahead and navigate.
" Use command ':verbose imap <tab>' to make sure tab is not mapped by other plugin.
inoremap <silent><expr> <TAB>
      \ pumvisible() ? "\<C-n>" :
      \ <SID>check_back_space() ? "\<TAB>" :
      \ coc#refresh()
inoremap <expr><S-TAB> pumvisible() ? "\<C-p>" : "\<C-h>"

function! s:check_back_space() abort
  let col = col('.') - 1
  return !col || getline('.')[col - 1]  =~# '\s'
endfunction

" Use tab for trigger completion with characters ahead and navigate.
" Use command ':verbose imap <tab>' to make sure tab is not mapped by other plugin.
inoremap <silent><expr> <TAB>
      \ pumvisible() ? "\<C-n>" :
      \ <SID>check_back_space() ? "\<TAB>" :
      \ coc#refresh()
inoremap <expr><S-TAB> pumvisible() ? "\<C-p>" : "\<C-h>"

function! s:check_back_space() abort
  let col = col('.') - 1
  return !col || getline('.')[col - 1]  =~# '\s'
endfunction
