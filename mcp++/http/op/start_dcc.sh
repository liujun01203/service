cd ../
pn=`pwd | xargs -i basename {}`
cd ./bin
kill -15 `cat $pn"_dcc.pid"` > /dev/null 2>&1
./$pn"_dcc" "../etc/"$pn"_dcc.conf"
