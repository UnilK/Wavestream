map mk :make<CR>

map <C-h> G/include<CR> :noh<CR> w w gf

set path=.,include

nmap qqq :qa<CR>

execute "set <C-S-M-q>=\eq"
execute "set <C-S-M-e>=\ee"
imap <C-S-M-q> :ptag <C-r><C-w><CR>
imap <C-S-M-e> :pc<CR>
nmap <C-S-M-q> :ptag <C-r><C-w><CR>
nmap <C-S-M-e> :pc<CR>

