ftp -n <<- EOF

open home.ustc.edu.cn

user username password

binary

cd /public_html

put index.html

put github-markdown.css

bye

EOF
