#!/bin/bash
find $1 -regex ".*\.\(c\|h\|hpp\|cc\|cpp\|pl\|pm\|t\|cgi\|inc\|py\|rb\|tmpl\|tt\|js\|coffee\|sql\|css\|scss\|php\)" > cscope.files

cscope -bqR
ctags -R --langmap=perl:.pl.t.inc.cgi.pm,javascript:.jc.coffee,python:.py,ruby:.rb,html:.tmpl.tt,sql:.sql,php:.php --languages=+c++,c,perl,java,python,ruby,javascript,html,sql,php --fields=+iaS --extra=+qf --links=no ../
