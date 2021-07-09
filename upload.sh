ftp -n <<- EOF

open home.ustc.edu.cn

user jzhang19 Z6543210

binary

cd /public_html

put index.html

put github-markdown.css

bye

EOF
