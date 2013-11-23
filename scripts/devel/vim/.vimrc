set nocompatible
syntax on             " Enable syntax highlighting
filetype on           " Enable filetype detection
filetype indent on    " Enable filetype-specific indenting
filetype plugin on    " Enable filetype-specific plugins

set nohlsearch

if &t_Co > 2 || has("gui_running")
  syntax on
  set hlsearch
endif

set ruler               " zobrazuje pozici kurzoru ve spodnim radku napravo
set rulerformat=%3b\ \ %l,%c%V%=%P
set showmode            " zobrazuje rezim INSERT/REPLACE/... ve spodnim radku
set showmatch           " kdyz napisete uzavirajici zavorku, ukaze vam to pocatecni
set number
set showcmd

set viminfo='50,\"500   "
set history=50

set incsearch           " pri vyhledavani prubezne ukazuje prvni shodu 
set ignorecase          " case insensitive search
set display=lastline
set scrolloff=5

set spl=cs

set wildchar=<Tab>
set wildmenu
set wildmode=longest:full,full

set backup
set backupdir=~/.vim/backup/

set autoindent
set smartindent
set expandtab          " hitting Tab in insert mode will or won'tt produce the appropriate number of spaces.
set smarttab
set cindent
set shiftwidth=2
set softtabstop=2
set tabstop=2
set shiftround
set nowrap

set backspace=indent,eol,start
set clipboard=unnamed
set laststatus=2
set statusline=%1*%n:%*%F %m%2*%r%*%=[%b,0x%B]\ \ %l/%L,%c%V\ \ %P
highlight User1 guibg=white guifg=blue
highlight User2 guibg=white guifg=red

colorscheme elflord

set tabpagemax=20

set listchars=eol:$,tab:>-,trail:·,extends:>,precedes:<
set list
hi NonText ctermfg=darkgray guifg=darkgray
hi clear SpecialKey
hi link SpecialKey NonText

set bk
